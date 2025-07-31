#pragma once

#include <string>
#include <unordered_map>

namespace r3m {
namespace quality {

struct QualityMetrics {
    double content_quality_score = 0.0;
    double information_density = 0.0;
    bool is_high_quality = false;
    std::string quality_reason;
    size_t text_length = 0;
    size_t unique_words = 0;
    size_t sentence_count = 0;
    size_t technical_terms = 0;
};

struct QualityConfig {
    bool enabled = true;
    double min_content_quality_score = 0.3;
    double min_information_density = 0.1;
    size_t min_content_length = 50;
    size_t max_content_length = 1000000;
    bool filter_empty_documents = true;
    bool filter_low_quality_documents = true;
};

class QualityAssessor {
public:
    QualityAssessor();
    ~QualityAssessor() = default;

    bool initialize(const std::unordered_map<std::string, std::string>& config);
    
    // Quality assessment
    QualityMetrics assess_quality(const std::string& text_content);
    bool filter_document(const QualityMetrics& metrics) const;
    bool is_high_quality_content(const QualityMetrics& metrics) const;
    
    // Quality calculations
    double calculate_content_quality_score(const std::string& text);
    double calculate_information_density(const std::string& text);
    
    // Configuration
    QualityConfig get_config() const { return config_; }

private:
    QualityConfig config_;
    
    // Helper methods for quality calculation
    size_t count_unique_words(const std::string& text);
    size_t count_sentences(const std::string& text);
    size_t count_technical_terms(const std::string& text);
    std::string determine_quality_reason(const QualityMetrics& metrics);
};

} // namespace quality
} // namespace r3m 