#include "r3m/chunking/quality_assessment/quality_calculator.hpp"

namespace r3m {
namespace chunking {
namespace quality_assessment {

double QualityCalculator::calculate_word_diversity(const std::string& text) {
    // Simple word diversity calculation
    std::unordered_set<std::string> unique_words;
    std::istringstream iss(text);
    std::string word;
    
    while (iss >> word) {
        unique_words.insert(word);
    }
    
    return std::min(1.0, static_cast<double>(unique_words.size()) / 100.0);
}

double QualityCalculator::calculate_sentence_structure(const std::string& text) {
    // Simple sentence structure calculation
    int sentences = 0;
    int words = 0;
    
    std::istringstream iss(text);
    std::string word;
    
    while (iss >> word) {
        words++;
        if (word.find('.') != std::string::npos || 
            word.find('!') != std::string::npos || 
            word.find('?') != std::string::npos) {
            sentences++;
        }
    }
    
    if (sentences == 0) return 0.0;
    double avg_sentence_length = static_cast<double>(words) / sentences;
    return std::min(1.0, avg_sentence_length / 20.0);
}

double QualityCalculator::calculate_information_density(const std::string& text) {
    // Simple information density calculation
    std::unordered_set<char> chars;
    
    for (char c : text) {
        if (std::isalnum(c)) {
            chars.insert(c);
        }
    }
    
    return std::min(1.0, static_cast<double>(chars.size()) / 50.0);
}

double QualityCalculator::calculate_quality_score(const std::string& text) {
    // Calculate quality score (0.0 - 1.0)
    double length_factor = std::min(1.0, static_cast<double>(text.length()) / 1000.0);
    double word_diversity = calculate_word_diversity(text);
    double sentence_structure = calculate_sentence_structure(text);
    double information_density = calculate_information_density(text);
    
    return (length_factor * 0.3 + word_diversity * 0.3 + 
            sentence_structure * 0.2 + information_density * 0.2);
}

} // namespace quality_assessment
} // namespace chunking
} // namespace r3m 