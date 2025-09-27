#include "../include/utils.h"

std::unordered_map<char, int> calculateFrequencies(const std::string& text) {
    std::unordered_map<char, int> freq;
    for (char c : text) {
        freq[c]++;
    }
    return freq;
}



