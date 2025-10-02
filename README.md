# Huffman Compressor

A C++ implementation of file compression and decompression using the Huffman coding algorithm.

![Huffman Coding](https://upload.wikimedia.org/wikipedia/commons/8/82/Huffman_tree_2.svg)

## 📋 Overview

This project provides a lightweight, efficient file compression and decompression tool based on Huffman coding. Huffman coding is a lossless data compression algorithm that assigns variable-length codes to input characters based on their frequencies - characters that occur more frequently are assigned shorter codes.

## ✨ Features

- **Lossless compression** - Original data can be perfectly reconstructed
- **Binary file support** - Works with any file type
- **Optimal encoding** - More frequent characters use fewer bits
- **Simple interface** - Easy-to-use command-line operation
- **Debug output** - Detailed information about compression statistics

## 🚀 Performance

Example compression results (your results may vary based on file content):

| File Type | Original Size | Compressed Size | Compression Ratio |
|-----------|--------------|----------------|------------------|
| Text file | 1000 bytes   | ~600 bytes     | ~40%             |
| Source code | 10 KB      | ~6 KB          | ~40%             |
| Binary file | 100 KB     | ~70 KB         | ~30%             |

## 📦 Installation

### Prerequisites

- C++ compiler with C++17 support
- Any modern operating system (Windows, macOS, Linux)

### Building from source

Clone the repository:

```bash
git clone https://github.com/NikSterPrime/Compressor.git
cd Compressor
```

Build the project:

```bash
# On Linux/macOS
g++ -std=c++17 -I include src/*.cpp -o compressor

# On Windows
g++ -std=c++17 -I include src/*.cpp -o compressor.exe
```

## 🔧 Usage

### Compressing a file

```bash
./compressor compress input.txt compressed.txt
```

### Decompressing a file

```bash
./compressor decompress compressed.txt decompressed.txt
```

## 🧠 How It Works

### Compression Process

1. **Frequency Analysis**: Count the frequency of each character in the input file
2. **Build Huffman Tree**: Create a binary tree where more frequent characters are closer to the root
3. **Generate Codes**: Create a unique binary code for each character based on its position in the tree
4. **Encode Data**: Convert the original file to a sequence of bits using these codes
5. **Write Output**: Store the compressed data and mapping table for later decompression

### Decompression Process

1. **Read Mapping**: Load the character-to-code mapping from the map file
2. **Rebuild Tree**: Reconstruct the Huffman tree from the mapping
3. **Decode Data**: Traverse the tree for each bit in the compressed data
4. **Write Output**: Output the original characters when leaf nodes are reached

## 📂 Project Structure

```
Compressor/
├── include/                 # Header files
│   ├── huffmanCompress.h    # Compression interface
│   ├── huffmanDecompress.h  # Decompression interface
│   └── utils.h              # Utility functions
├── src/                     # Source files
│   ├── huffmanCompress.cpp  # Compression implementation
│   ├── huffmanDecompress.cpp # Decompression implementation
│   ├── main.cpp             # Entry point
│   └── utils.cpp            # Utility implementations
├── data/                    # Example data files
└── test/                    # Test files and examples
```

## 🤝 Contributing

Contributions, issues, and feature requests are welcome! Feel free to check the [issues page](https://github.com/NikSterPrime/Compressor/issues).

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgements

- [Huffman coding on Wikipedia](https://en.wikipedia.org/wiki/Huffman_coding)
- The image of the Huffman tree is from Wikipedia (CC BY-SA 3.0)