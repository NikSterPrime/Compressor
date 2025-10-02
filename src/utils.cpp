#include "../include/utils.h"
#include <map>

std::map<char, int> calculateFrequencies(const std::string& text) {
    std::map<char, int> freq;
    for (char c : text) {
        freq[c]++;
    }
    return freq;
}



