#include "string_processing.h"

std::vector<std::string> SplitIntoWords(const std::string_view text) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == ' ') {
            words.push_back(word);
            word.clear();
        }
        else {
            word += c;
        }
    }
    words.push_back(word);

    return words;
}