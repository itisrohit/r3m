#pragma once

#include <string>
#include <unordered_set>
#include <sstream>
#include <cctype>
#include <algorithm>

namespace r3m {
namespace chunking {
namespace quality_assessment {

/**
 * @brief Quality calculator for document chunks
 * 
 * Provides methods to calculate various quality metrics for document chunks,
 * including word diversity, sentence structure, and information density.
 */
class QualityCalculator {
public:
    /**
     * @brief Calculate word diversity score
     * @param text The text to analyze
     * @return Diversity score between 0.0 and 1.0
     */
    static double calculate_word_diversity(const std::string& text);
    
    /**
     * @brief Calculate sentence structure score
     * @param text The text to analyze
     * @return Sentence structure score between 0.0 and 1.0
     */
    static double calculate_sentence_structure(const std::string& text);
    
    /**
     * @brief Calculate information density score
     * @param text The text to analyze
     * @return Information density score between 0.0 and 1.0
     */
    static double calculate_information_density(const std::string& text);
    
    /**
     * @brief Calculate overall quality score
     * @param text The text to analyze
     * @return Overall quality score between 0.0 and 1.0
     */
    static double calculate_quality_score(const std::string& text);
};

} // namespace quality_assessment
} // namespace chunking
} // namespace r3m 