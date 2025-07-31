#include "r3m/quality/assessor.hpp"
#include "r3m/utils/text_utils.hpp"

#include <sstream>
#include <algorithm>
#include <set>
#include <cmath>

namespace r3m {
namespace quality {

QualityAssessor::QualityAssessor() {
    // Default configuration is set in the header
}

bool QualityAssessor::initialize(const std::unordered_map<std::string, std::string>& config) {
    // Load quality filtering settings
    auto it = config.find("document_processing.quality_filtering.enabled");
    if (it != config.end()) {
        config_.enabled = (it->second == "true");
    }
    
    it = config.find("document_processing.quality_filtering.min_content_quality_score");
    if (it != config.end()) {
        config_.min_content_quality_score = std::stod(it->second);
    }
    
    it = config.find("document_processing.quality_filtering.min_information_density");
    if (it != config.end()) {
        config_.min_information_density = std::stod(it->second);
    }
    
    it = config.find("document_processing.quality_filtering.min_content_length");
    if (it != config.end()) {
        config_.min_content_length = std::stoul(it->second);
    }
    
    it = config.find("document_processing.quality_filtering.max_content_length");
    if (it != config.end()) {
        config_.max_content_length = std::stoul(it->second);
    }
    
    it = config.find("document_processing.quality_filtering.filter_empty_documents");
    if (it != config.end()) {
        config_.filter_empty_documents = (it->second == "true");
    }
    
    it = config.find("document_processing.quality_filtering.filter_low_quality_documents");
    if (it != config.end()) {
        config_.filter_low_quality_documents = (it->second == "true");
    }
    
    return true;
}

QualityMetrics QualityAssessor::assess_quality(const std::string& text_content) {
    QualityMetrics metrics;
    
    metrics.text_length = text_content.length();
    metrics.unique_words = count_unique_words(text_content);
    metrics.sentence_count = count_sentences(text_content);
    metrics.technical_terms = count_technical_terms(text_content);
    
    // Calculate quality scores
    metrics.content_quality_score = calculate_content_quality_score(text_content);
    metrics.information_density = calculate_information_density(text_content);
    metrics.is_high_quality = is_high_quality_content(metrics);
    metrics.quality_reason = determine_quality_reason(metrics);
    
    return metrics;
}

bool QualityAssessor::filter_document(const QualityMetrics& metrics) const {
    if (!config_.enabled) {
        return true; // No filtering
    }
    
    // Check if document should be filtered out
    if (config_.filter_empty_documents && metrics.text_length == 0) {
        return false;
    }
    
    if (metrics.text_length < config_.min_content_length) {
        return false;
    }
    
    if (metrics.text_length > config_.max_content_length) {
        return false;
    }
    
    if (config_.filter_low_quality_documents && !metrics.is_high_quality) {
        return false;
    }
    
    return true;
}

bool QualityAssessor::is_high_quality_content(const QualityMetrics& metrics) const {
    return metrics.content_quality_score >= config_.min_content_quality_score &&
           metrics.information_density >= config_.min_information_density &&
           metrics.text_length >= config_.min_content_length &&
           metrics.text_length <= config_.max_content_length;
}

double QualityAssessor::calculate_content_quality_score(const std::string& text) {
    if (text.empty()) {
        return 0.0;
    }
    
    // Simple quality scoring based on content characteristics
    double score = 0.0;
    
    // Length factor (0-0.3)
    double length_factor = std::min(1.0, static_cast<double>(text.length()) / 1000.0);
    score += length_factor * 0.3;
    
    // Word diversity factor (0-0.3)
    std::set<std::string> unique_words = utils::TextUtils::get_unique_words(text);
    double word_diversity = static_cast<double>(unique_words.size()) / std::max(1.0, static_cast<double>(text.length() / 5.0));
    score += std::min(1.0, word_diversity) * 0.3;
    
    // Sentence structure factor (0-0.2)
    size_t sentence_count = utils::TextUtils::count_sentences(text);
    double sentence_factor = std::min(1.0, static_cast<double>(sentence_count) / 10.0);
    score += sentence_factor * 0.2;
    
    // Information density factor (0-0.2)
    double info_density = calculate_information_density(text);
    score += info_density * 0.2;
    
    return std::min(1.0, std::max(0.0, score));
}

double QualityAssessor::calculate_information_density(const std::string& text) {
    if (text.empty()) {
        return 0.0;
    }
    
    // Calculate information density based on content characteristics
    double density = 0.0;
    
    // Unique word ratio
    std::set<std::string> unique_words = utils::TextUtils::get_unique_words(text);
    double unique_word_ratio = static_cast<double>(unique_words.size()) / std::max(1.0, static_cast<double>(text.length() / 5.0));
    density += unique_word_ratio * 0.4;
    
    // Technical term density (words with numbers, special characters)
    size_t technical_terms = utils::TextUtils::count_technical_terms(text);
    double technical_density = static_cast<double>(technical_terms) / std::max(1.0, static_cast<double>(text.length() / 10.0));
    density += technical_density * 0.3;
    
    // Sentence complexity (average sentence length)
    size_t sentences = utils::TextUtils::count_sentences(text);
    if (sentences > 0) {
        double avg_sentence_length = static_cast<double>(text.length()) / sentences;
        double complexity_factor = std::min(1.0, avg_sentence_length / 100.0);
        density += complexity_factor * 0.3;
    }
    
    return std::min(1.0, std::max(0.0, density));
}

size_t QualityAssessor::count_unique_words(const std::string& text) {
    return utils::TextUtils::get_unique_words(text).size();
}

size_t QualityAssessor::count_sentences(const std::string& text) {
    return utils::TextUtils::count_sentences(text);
}

size_t QualityAssessor::count_technical_terms(const std::string& text) {
    return utils::TextUtils::count_technical_terms(text);
}

std::string QualityAssessor::determine_quality_reason(const QualityMetrics& metrics) {
    if (metrics.is_high_quality) {
        return "High quality content";
    } else if (metrics.text_length < config_.min_content_length) {
        return "Content too short";
    } else if (metrics.content_quality_score < config_.min_content_quality_score) {
        return "Low content quality score";
    } else if (metrics.information_density < config_.min_information_density) {
        return "Low information density";
    } else {
        return "Quality assessment failed";
    }
}

} // namespace quality
} // namespace r3m 