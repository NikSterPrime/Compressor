#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <cstring>
#include <omp.h>

// ============================================================================
// RLE COMPRESSION FUNCTIONS
// ============================================================================

/**
 * Sequential RLE compression
 * Returns compressed data with format: [count][character]
 */
std::vector<uint8_t> rle_compress_sequential(const std::vector<uint8_t>& input) {
    std::vector<uint8_t> output;
    if (input.empty()) return output;
    
    size_t i = 0;
    while (i < input.size()) {
        uint8_t current = input[i];
        size_t count = 1;
        
        // Count consecutive occurrences (max 255)
        while (i + count < input.size() && 
               input[i + count] == current && 
               count < 255) {
            count++;
        }
        
        // Store count and character
        output.push_back(static_cast<uint8_t>(count));
        output.push_back(current);
        
        i += count;
    }
    
    return output;
}

/**
 * Parallel RLE compression using chunking
 * Divides input into chunks and compresses in parallel
 */
std::vector<uint8_t> rle_compress_parallel(const std::vector<uint8_t>& input, 
                                           int num_threads = 4) {
    if (input.empty()) return std::vector<uint8_t>();
    
    size_t chunk_size = input.size() / num_threads;
    if (chunk_size == 0) {
        return rle_compress_sequential(input);
    }
    
    std::vector<std::vector<uint8_t>> partial_results(num_threads);
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        size_t start = tid * chunk_size;
        size_t end = (tid == num_threads - 1) ? input.size() : (tid + 1) * chunk_size;
        
        // Compress chunk
        std::vector<uint8_t> chunk_compressed;
        size_t i = start;
        
        while (i < end) {
            uint8_t current = input[i];
            size_t count = 1;
            
            // Count consecutive occurrences
            while (i + count < end && 
                   input[i + count] == current && 
                   count < 255) {
                count++;
            }
            
            chunk_compressed.push_back(static_cast<uint8_t>(count));
            chunk_compressed.push_back(current);
            
            i += count;
        }
        
        partial_results[tid] = chunk_compressed;
    }
    
    // Merge results sequentially
    std::vector<uint8_t> output;
    for (const auto& partial : partial_results) {
        output.insert(output.end(), partial.begin(), partial.end());
    }
    
    return output;
}

// ============================================================================
// RLE DECOMPRESSION FUNCTIONS
// ============================================================================

/**
 * Sequential RLE decompression
 */
std::vector<uint8_t> rle_decompress_sequential(const std::vector<uint8_t>& input) {
    std::vector<uint8_t> output;
    
    for (size_t i = 0; i + 1 < input.size(); i += 2) {
        uint8_t count = input[i];
        uint8_t value = input[i + 1];
        
        for (uint8_t j = 0; j < count; j++) {
            output.push_back(value);
        }
    }
    
    return output;
}

/**
 * Parallel RLE decompression
 * Analyzes compressed data structure first, then decompresses in parallel
 */
std::vector<uint8_t> rle_decompress_parallel(const std::vector<uint8_t>& input, 
                                             int num_threads = 4) {
    if (input.size() < 2) return std::vector<uint8_t>();
    
    // Phase 1: Calculate total output size and chunk boundaries
    std::vector<size_t> positions;
    std::vector<size_t> output_positions;
    size_t total_size = 0;
    
    for (size_t i = 0; i + 1 < input.size(); i += 2) {
        positions.push_back(i);
        output_positions.push_back(total_size);
        total_size += input[i];
    }
    
    std::vector<uint8_t> output(total_size);
    
    // Phase 2: Parallel decompression
    #pragma omp parallel for num_threads(num_threads) schedule(dynamic)
    for (size_t idx = 0; idx < positions.size(); idx++) {
        size_t i = positions[idx];
        size_t out_pos = output_positions[idx];
        
        uint8_t count = input[i];
        uint8_t value = input[i + 1];
        
        for (uint8_t j = 0; j < count; j++) {
            output[out_pos + j] = value;
        }
    }
    
    return output;
}

// ============================================================================
// PIPELINE PARALLELISM IMPLEMENTATION
// ============================================================================

/**
 * Pipeline structure for producer-consumer pattern
 * Stage 1: Read data (I/O)
 * Stage 2: Compress/Decompress (Compute)
 * Stage 3: Write data (I/O)
 */
class CompressionPipeline {
private:
    struct DataBuffer {
        std::vector<uint8_t> data;
        bool valid;
        bool end_of_stream;
        
        DataBuffer() : valid(false), end_of_stream(false) {}
    };
    
    std::vector<DataBuffer> buffers;
    int buffer_count;
    int read_index;
    int process_index;
    int write_index;
    
public:
    CompressionPipeline(int num_buffers = 4) 
        : buffer_count(num_buffers), read_index(0), 
          process_index(0), write_index(0) {
        buffers.resize(num_buffers);
    }
    
    /**
     * Pipeline compression with 3 stages running concurrently
     */
    void compress_pipeline(const std::string& input_file, 
                          const std::string& output_file,
                          size_t chunk_size = 1024 * 1024) {
        
        double io_time = 0.0;
        double compute_time = 0.0;
        
        #pragma omp parallel sections num_threads(3)
        {
            // Stage 1: Producer - Read from file (I/O bound)
            #pragma omp section
            {
                std::ifstream infile(input_file, std::ios::binary);
                std::vector<uint8_t> buffer(chunk_size);
                
                while (infile) {
                    auto start = std::chrono::high_resolution_clock::now();
                    
                    infile.read(reinterpret_cast<char*>(buffer.data()), chunk_size);
                    size_t bytes_read = infile.gcount();
                    
                    auto end = std::chrono::high_resolution_clock::now();
                    
                    #pragma omp atomic
                    io_time += std::chrono::duration<double>(end - start).count();
                    
                    if (bytes_read > 0) {
                        // Wait for buffer to be available
                        #pragma omp critical (buffer_write)
                        {
                            buffers[read_index % buffer_count].data.assign(
                                buffer.begin(), buffer.begin() + bytes_read);
                            buffers[read_index % buffer_count].valid = true;
                            buffers[read_index % buffer_count].end_of_stream = false;
                            read_index++;
                        }
                    }
                    
                    if (bytes_read < chunk_size) break;
                }
                
                // Signal end of stream
                #pragma omp critical (buffer_write)
                {
                    buffers[read_index % buffer_count].end_of_stream = true;
                    buffers[read_index % buffer_count].valid = true;
                }
                
                infile.close();
            }
            
            // Stage 2: Worker - Compress data (Compute bound)
            #pragma omp section
            {
                bool done = false;
                
                while (!done) {
                    bool has_data = false;
                    std::vector<uint8_t> input_data;
                    
                    #pragma omp critical (buffer_write)
                    {
                        if (buffers[process_index % buffer_count].valid) {
                            if (buffers[process_index % buffer_count].end_of_stream) {
                                done = true;
                            } else {
                                input_data = buffers[process_index % buffer_count].data;
                                has_data = true;
                                buffers[process_index % buffer_count].valid = false;
                            }
                            process_index++;
                        }
                    }
                    
                    if (has_data) {
                        auto start = std::chrono::high_resolution_clock::now();
                        
                        // Perform compression (compute intensive)
                        std::vector<uint8_t> compressed = rle_compress_sequential(input_data);
                        
                        auto end = std::chrono::high_resolution_clock::now();
                        
                        #pragma omp atomic
                        compute_time += std::chrono::duration<double>(end - start).count();
                        
                        // Store compressed data for writing
                        #pragma omp critical (output_queue)
                        {
                            // In a real implementation, use a separate output queue
                            // For simplicity, we're demonstrating the pattern
                        }
                    }
                }
            }
            
            // Stage 3: Consumer - Write to file (I/O bound)
            #pragma omp section
            {
                std::ofstream outfile(output_file, std::ios::binary);
                // Write compressed data as it becomes available
                // Implementation details omitted for brevity
                outfile.close();
            }
        }
        
        // Print bottleneck analysis
        std::cout << "\n=== BOTTLENECK ANALYSIS ===" << std::endl;
        std::cout << "I/O Time:      " << io_time << " seconds" << std::endl;
        std::cout << "Compute Time:  " << compute_time << " seconds" << std::endl;
        
        if (io_time > compute_time * 1.5) {
            std::cout << "BOTTLENECK: I/O bound (Consider buffering, async I/O)" << std::endl;
        } else if (compute_time > io_time * 1.5) {
            std::cout << "BOTTLENECK: Compute bound (Consider more parallelism)" << std::endl;
        } else {
            std::cout << "BALANCED: I/O and compute are well balanced" << std::endl;
        }
    }
};

// ============================================================================
// PERFORMANCE BENCHMARKING
// ============================================================================

void benchmark_compression(const std::vector<uint8_t>& data, int num_threads) {
    std::cout << "\n=== RLE Compression Benchmark ===" << std::endl;
    std::cout << "Input size: " << data.size() << " bytes" << std::endl;
    std::cout << "Threads: " << num_threads << std::endl;
    
    // Sequential compression
    auto start = std::chrono::high_resolution_clock::now();
    auto compressed_seq = rle_compress_sequential(data);
    auto end = std::chrono::high_resolution_clock::now();
    double time_seq = std::chrono::duration<double>(end - start).count();
    
    std::cout << "\nSequential:" << std::endl;
    std::cout << "  Time: " << time_seq << " seconds" << std::endl;
    std::cout << "  Output size: " << compressed_seq.size() << " bytes" << std::endl;
    std::cout << "  Compression ratio: " << 
        (double)data.size() / compressed_seq.size() << std::endl;
    
    // Parallel compression
    start = std::chrono::high_resolution_clock::now();
    auto compressed_par = rle_compress_parallel(data, num_threads);
    end = std::chrono::high_resolution_clock::now();
    double time_par = std::chrono::duration<double>(end - start).count();
    
    std::cout << "\nParallel (" << num_threads << " threads):" << std::endl;
    std::cout << "  Time: " << time_par << " seconds" << std::endl;
    std::cout << "  Output size: " << compressed_par.size() << " bytes" << std::endl;
    std::cout << "  Speedup: " << time_seq / time_par << "x" << std::endl;
    std::cout << "  Efficiency: " << 
        (time_seq / time_par) / num_threads * 100 << "%" << std::endl;
}

void benchmark_decompression(const std::vector<uint8_t>& compressed, int num_threads) {
    std::cout << "\n=== RLE Decompression Benchmark ===" << std::endl;
    std::cout << "Compressed size: " << compressed.size() << " bytes" << std::endl;
    
    // Sequential decompression
    auto start = std::chrono::high_resolution_clock::now();
    auto decompressed_seq = rle_decompress_sequential(compressed);
    auto end = std::chrono::high_resolution_clock::now();
    double time_seq = std::chrono::duration<double>(end - start).count();
    
    std::cout << "\nSequential:" << std::endl;
    std::cout << "  Time: " << time_seq << " seconds" << std::endl;
    std::cout << "  Output size: " << decompressed_seq.size() << " bytes" << std::endl;
    
    // Parallel decompression
    start = std::chrono::high_resolution_clock::now();
    auto decompressed_par = rle_decompress_parallel(compressed, num_threads);
    end = std::chrono::high_resolution_clock::now();
    double time_par = std::chrono::duration<double>(end - start).count();
    
    std::cout << "\nParallel (" << num_threads << " threads):" << std::endl;
    std::cout << "  Time: " << time_par << " seconds" << std::endl;
    std::cout << "  Speedup: " << time_seq / time_par << "x" << std::endl;
    std::cout << "  Efficiency: " << 
        (time_seq / time_par) / num_threads * 100 << "%" << std::endl;
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main(int argc, char* argv[]) {
    std::cout << "RLE Compression/Decompression with OpenMP" << std::endl;
    std::cout << "==========================================\n" << std::endl;
    
    // Generate test data with repetitive patterns (good for RLE)
    const size_t data_size = 10 * 1024 * 1024; // 10 MB
    std::vector<uint8_t> test_data(data_size);
    
    // Create data with runs of repeated values
    for (size_t i = 0; i < data_size; i++) {
        test_data[i] = static_cast<uint8_t>((i / 100) % 256);
    }
    
    std::cout << "Generated " << data_size << " bytes of test data" << std::endl;
    
    // Test different thread counts
    std::vector<int> thread_counts = {1, 2, 4, 8};
    
    for (int num_threads : thread_counts) {
        std::cout << "\n" << std::string(50, '=') << std::endl;
        
        // Benchmark compression
        benchmark_compression(test_data, num_threads);
        
        // Create compressed data for decompression test
        auto compressed = rle_compress_sequential(test_data);
        
        // Benchmark decompression
        benchmark_decompression(compressed, num_threads);
    }
    
    // Demonstrate pipeline parallelism
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "\n=== Pipeline Parallelism Demo ===" << std::endl;
    
    // Save test data to file
    std::ofstream test_file("test_input.dat", std::ios::binary);
    test_file.write(reinterpret_cast<const char*>(test_data.data()), test_data.size());
    test_file.close();
    
    // Run pipeline compression
    CompressionPipeline pipeline;
    pipeline.compress_pipeline("test_input.dat", "test_output.rle", 1024 * 1024);
    
    return 0;
}
