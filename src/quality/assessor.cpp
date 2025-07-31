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
    config_map_ = config;
    
    // Load quality filtering configuration
    auto it = config_map_.find("document_processing.quality_filtering.enabled");
    if (it != config_map_.end()) {
        config_.enabled = (it->second == "true");
    }
    
    it = config_map_.find("document_processing.quality_filtering.min_content_quality_score");
    if (it != config_map_.end()) {
        config_.min_content_quality_score = std::stod(it->second);
    }
    
    it = config_map_.find("document_processing.quality_filtering.min_information_density");
    if (it != config_map_.end()) {
        config_.min_information_density = std::stod(it->second);
    }
    
    it = config_map_.find("document_processing.quality_filtering.min_content_length");
    if (it != config_map_.end()) {
        config_.min_content_length = std::stoul(it->second);
    }
    
    it = config_map_.find("document_processing.quality_filtering.max_content_length");
    if (it != config_map_.end()) {
        config_.max_content_length = std::stoul(it->second);
    }
    
    it = config_map_.find("document_processing.quality_filtering.filter_empty_documents");
    if (it != config_map_.end()) {
        config_.filter_empty_documents = (it->second == "true");
    }
    
    it = config_map_.find("document_processing.quality_filtering.filter_low_quality_documents");
    if (it != config_map_.end()) {
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
    
    // Get weights from config (with defaults)
    double length_weight = 0.3;
    double word_diversity_weight = 0.3;
    double sentence_structure_weight = 0.2;
    double info_density_weight = 0.2;
    
    auto it = config_map_.find("document_processing.quality_filtering.quality_weights.length_factor");
    if (it != config_map_.end()) {
        length_weight = std::stod(it->second);
    }
    
    it = config_map_.find("document_processing.quality_filtering.quality_weights.word_diversity_factor");
    if (it != config_map_.end()) {
        word_diversity_weight = std::stod(it->second);
    }
    
    it = config_map_.find("document_processing.quality_filtering.quality_weights.sentence_structure_factor");
    if (it != config_map_.end()) {
        sentence_structure_weight = std::stod(it->second);
    }
    
    it = config_map_.find("document_processing.quality_filtering.quality_weights.information_density_factor");
    if (it != config_map_.end()) {
        info_density_weight = std::stod(it->second);
    }
    
    // Get thresholds from config (with defaults)
    double length_normalization = 1000.0;
    double word_diversity_normalization = 5.0;
    double sentence_normalization = 10.0;
    
    it = config_map_.find("document_processing.quality_filtering.quality_thresholds.length_normalization");
    if (it != config_map_.end()) {
        length_normalization = std::stod(it->second);
    }
    
    it = config_map_.find("document_processing.quality_filtering.quality_thresholds.word_diversity_normalization");
    if (it != config_map_.end()) {
        word_diversity_normalization = std::stod(it->second);
    }
    
    it = config_map_.find("document_processing.quality_filtering.quality_thresholds.sentence_normalization");
    if (it != config_map_.end()) {
        sentence_normalization = std::stod(it->second);
    }
    
    // Length factor
    double length_factor = std::min(1.0, static_cast<double>(text.length()) / length_normalization);
    score += length_factor * length_weight;
    
    // Word diversity factor
    std::set<std::string> unique_words = utils::TextUtils::get_unique_words(text);
    double word_diversity = static_cast<double>(unique_words.size()) / std::max(1.0, static_cast<double>(text.length() / word_diversity_normalization));
    score += std::min(1.0, word_diversity) * word_diversity_weight;
    
    // Sentence structure factor
    size_t sentence_count = utils::TextUtils::count_sentences(text);
    double sentence_factor = std::min(1.0, static_cast<double>(sentence_count) / sentence_normalization);
    score += sentence_factor * sentence_structure_weight;
    
    // Information density factor
    double info_density = calculate_information_density(text);
    score += info_density * info_density_weight;
    
    return std::min(1.0, std::max(0.0, score));
}

double QualityAssessor::calculate_information_density(const std::string& text) {
    if (text.empty()) {
        return 0.0;
    }
    
    // Get weights from config (with defaults)
    double unique_word_ratio_weight = 0.4;
    double technical_term_density_weight = 0.3;
    double sentence_complexity_weight = 0.3;
    
    auto it = config_map_.find("document_processing.quality_filtering.density_weights.unique_word_ratio");
    if (it != config_map_.end()) {
        unique_word_ratio_weight = std::stod(it->second);
    }
    
    it = config_map_.find("document_processing.quality_filtering.density_weights.technical_term_density");
    if (it != config_map_.end()) {
        technical_term_density_weight = std::stod(it->second);
    }
    
    it = config_map_.find("document_processing.quality_filtering.density_weights.sentence_complexity");
    if (it != config_map_.end()) {
        sentence_complexity_weight = std::stod(it->second);
    }
    
    // Get thresholds from config (with defaults)
    double word_diversity_normalization = 5.0;
    double technical_term_normalization = 10.0;
    double sentence_complexity_normalization = 100.0;
    
    it = config_map_.find("document_processing.quality_filtering.quality_thresholds.word_diversity_normalization");
    if (it != config_map_.end()) {
        word_diversity_normalization = std::stod(it->second);
    }
    
    it = config_map_.find("document_processing.quality_filtering.quality_thresholds.technical_term_normalization");
    if (it != config_map_.end()) {
        technical_term_normalization = std::stod(it->second);
    }
    
    it = config_map_.find("document_processing.quality_filtering.quality_thresholds.sentence_complexity_normalization");
    if (it != config_map_.end()) {
        sentence_complexity_normalization = std::stod(it->second);
    }
    
    // Calculate information density based on content characteristics
    double density = 0.0;
    
    // Unique word ratio
    std::set<std::string> unique_words = utils::TextUtils::get_unique_words(text);
    double unique_word_ratio = static_cast<double>(unique_words.size()) / std::max(1.0, static_cast<double>(text.length() / word_diversity_normalization));
    density += unique_word_ratio * unique_word_ratio_weight;
    
    // Technical term density (words with numbers, special characters)
    size_t technical_terms = utils::TextUtils::count_technical_terms(text);
    double technical_density = static_cast<double>(technical_terms) / std::max(1.0, static_cast<double>(text.length() / technical_term_normalization));
    density += technical_density * technical_term_density_weight;
    
    // Sentence complexity (average sentence length)
    size_t sentences = utils::TextUtils::count_sentences(text);
    if (sentences > 0) {
        double avg_sentence_length = static_cast<double>(text.length()) / sentences;
        double complexity_factor = std::min(1.0, avg_sentence_length / sentence_complexity_normalization);
        density += complexity_factor * sentence_complexity_weight;
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