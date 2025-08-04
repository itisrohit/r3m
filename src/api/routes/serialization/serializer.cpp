#include "r3m/api/routes/serialization/serializer.hpp"
#include "r3m/api/routes/json_utils/json_utils.hpp"
#include "r3m/api/routes/response_handler/response_handler.hpp"
#include "r3m/core/document_processor.hpp"
#include <algorithm>

namespace r3m {
namespace api {
namespace serialization {

std::string serialize_document_result(const core::DocumentResult& result) {
    std::string response_data = "{\"job_id\":\"" + response_handler::generate_job_id() + "\",";
    response_data += "\"file_name\":\"" + result.file_name + "\",";
    response_data += "\"processing_success\":" + std::string(result.processing_success ? "true" : "false") + ",";
    response_data += "\"processing_time_ms\":" + std::to_string(result.processing_time_ms) + ",";
    response_data += "\"text_length\":" + std::to_string(result.text_content.length()) + ",";
    response_data += "\"content_quality_score\":" + std::to_string(result.content_quality_score) + ",";
    response_data += "\"information_density\":" + std::to_string(result.information_density) + ",";
    response_data += "\"is_high_quality\":" + std::string(result.is_high_quality ? "true" : "false") + ",";
    response_data += "\"quality_reason\":\"" + result.quality_reason + "\"}";
    
    return response_data;
}

std::string serialize_document_result_with_chunks(const core::DocumentResult& result) {
    std::string response_data = "{\"job_id\":\"" + response_handler::generate_job_id() + "\",";
    response_data += "\"file_name\":\"" + result.file_name + "\",";
    response_data += "\"processing_success\":" + std::string(result.processing_success ? "true" : "false") + ",";
    response_data += "\"processing_time_ms\":" + std::to_string(result.processing_time_ms) + ",";
    response_data += "\"text_length\":" + std::to_string(result.text_content.length()) + ",";
    response_data += "\"content_quality_score\":" + std::to_string(result.content_quality_score) + ",";
    response_data += "\"information_density\":" + std::to_string(result.information_density) + ",";
    response_data += "\"is_high_quality\":" + std::string(result.is_high_quality ? "true" : "false") + ",";
    response_data += "\"quality_reason\":\"" + result.quality_reason + "\",";
    response_data += "\"total_chunks\":" + std::to_string(result.total_chunks) + ",";
    response_data += "\"successful_chunks\":" + std::to_string(result.successful_chunks) + ",";
    response_data += "\"avg_chunk_quality\":" + std::to_string(result.avg_chunk_quality) + ",";
    response_data += "\"avg_chunk_density\":" + std::to_string(result.avg_chunk_density) + ",";
    response_data += "\"chunks\":[";
    
    bool first_chunk = true;
    for (const auto& chunk : result.chunks) {
        if (!first_chunk) response_data += ",";
        response_data += "{";
        response_data += "\"chunk_id\":" + std::to_string(chunk.chunk_id) + ",";
        response_data += "\"content\":\"" + json_utils::escape_json_string(chunk.content) + "\",";
        response_data += "\"blurb\":\"" + json_utils::escape_json_string(chunk.blurb) + "\",";
        response_data += "\"title_prefix\":\"" + json_utils::escape_json_string(chunk.title_prefix) + "\",";
        response_data += "\"metadata_suffix_semantic\":\"" + json_utils::escape_json_string(chunk.metadata_suffix_semantic) + "\",";
        response_data += "\"metadata_suffix_keyword\":\"" + json_utils::escape_json_string(chunk.metadata_suffix_keyword) + "\",";
        response_data += "\"quality_score\":" + std::to_string(chunk.quality_score) + ",";
        response_data += "\"information_density\":" + std::to_string(chunk.information_density) + ",";
        response_data += "\"is_high_quality\":" + std::string(chunk.is_high_quality ? "true" : "false") + ",";
        response_data += "\"title_tokens\":" + std::to_string(chunk.title_tokens) + ",";
        response_data += "\"metadata_tokens\":" + std::to_string(chunk.metadata_tokens) + ",";
        response_data += "\"content_token_limit\":" + std::to_string(chunk.content_token_limit) + "}";
        first_chunk = false;
    }
    response_data += "]}";
    
    return response_data;
}

std::string serialize_batch_results(const std::vector<core::DocumentResult>& results) {
    std::string response_data = "{\"total_files\":" + std::to_string(results.size()) + ",";
    response_data += "\"successful_processing\":" + std::to_string(std::count_if(results.begin(), results.end(),
        [](const core::DocumentResult& r) { return r.processing_success; })) + ",";
    response_data += "\"results\":[";
    
    bool first = true;
    for (const auto& result : results) {
        if (!first) response_data += ",";
        response_data += "{";
        response_data += "\"file_name\":\"" + result.file_name + "\",";
        response_data += "\"processing_success\":" + std::string(result.processing_success ? "true" : "false") + ",";
        response_data += "\"processing_time_ms\":" + std::to_string(result.processing_time_ms) + ",";
        response_data += "\"text_length\":" + std::to_string(result.text_content.length()) + ",";
        response_data += "\"content_quality_score\":" + std::to_string(result.content_quality_score) + ",";
        response_data += "\"information_density\":" + std::to_string(result.information_density) + ",";
        response_data += "\"is_high_quality\":" + std::string(result.is_high_quality ? "true" : "false");
        response_data += "}";
        first = false;
    }
    response_data += "]}";
    
    return response_data;
}

std::string serialize_batch_results_with_chunks(const std::vector<core::DocumentResult>& results) {
    std::string response_data = "{\"total_files\":" + std::to_string(results.size()) + ",";
    response_data += "\"successful_processing\":" + std::to_string(std::count_if(results.begin(), results.end(),
        [](const core::DocumentResult& r) { return r.processing_success; })) + ",";
    response_data += "\"results\":[";
    
    bool first_result = true;
    for (const auto& result : results) {
        if (!first_result) response_data += ",";
        response_data += "{";
        response_data += "\"file_name\":\"" + result.file_name + "\",";
        response_data += "\"processing_success\":" + std::string(result.processing_success ? "true" : "false") + ",";
        response_data += "\"processing_time_ms\":" + std::to_string(result.processing_time_ms) + ",";
        response_data += "\"text_length\":" + std::to_string(result.text_content.length()) + ",";
        response_data += "\"content_quality_score\":" + std::to_string(result.content_quality_score) + ",";
        response_data += "\"information_density\":" + std::to_string(result.information_density) + ",";
        response_data += "\"is_high_quality\":" + std::string(result.is_high_quality ? "true" : "false") + ",";
        response_data += "\"quality_reason\":\"" + result.quality_reason + "\",";
        response_data += "\"total_chunks\":" + std::to_string(result.total_chunks) + ",";
        response_data += "\"successful_chunks\":" + std::to_string(result.successful_chunks) + ",";
        response_data += "\"avg_chunk_quality\":" + std::to_string(result.avg_chunk_quality) + ",";
        response_data += "\"avg_chunk_density\":" + std::to_string(result.avg_chunk_density) + ",";
        response_data += "\"chunks\":[";
        
        bool first_chunk = true;
        for (const auto& chunk : result.chunks) {
            if (!first_chunk) response_data += ",";
            response_data += "{";
            response_data += "\"chunk_id\":" + std::to_string(chunk.chunk_id) + ",";
            response_data += "\"content\":\"" + json_utils::escape_json_string(chunk.content) + "\",";
            response_data += "\"blurb\":\"" + json_utils::escape_json_string(chunk.blurb) + "\",";
            response_data += "\"title_prefix\":\"" + json_utils::escape_json_string(chunk.title_prefix) + "\",";
            response_data += "\"metadata_suffix_semantic\":\"" + json_utils::escape_json_string(chunk.metadata_suffix_semantic) + "\",";
            response_data += "\"metadata_suffix_keyword\":\"" + json_utils::escape_json_string(chunk.metadata_suffix_keyword) + "\",";
            response_data += "\"quality_score\":" + std::to_string(chunk.quality_score) + ",";
            response_data += "\"information_density\":" + std::to_string(chunk.information_density) + ",";
            response_data += "\"is_high_quality\":" + std::string(chunk.is_high_quality ? "true" : "false") + ",";
            response_data += "\"title_tokens\":" + std::to_string(chunk.title_tokens) + ",";
            response_data += "\"metadata_tokens\":" + std::to_string(chunk.metadata_tokens) + ",";
            response_data += "\"content_token_limit\":" + std::to_string(chunk.content_token_limit) + "}";
            first_chunk = false;
        }
        response_data += "]}";
        first_result = false;
    }
    response_data += "]}";
    
    return response_data;
}

std::string serialize_chunking_result(const chunking::ChunkingResult& result) {
    std::string response_data = "{\"job_id\":\"" + response_handler::generate_job_id() + "\",";
    response_data += "\"total_chunks\":" + std::to_string(result.total_chunks) + ",";
    response_data += "\"successful_chunks\":" + std::to_string(result.successful_chunks) + ",";
    response_data += "\"failed_chunks\":" + std::to_string(result.failed_chunks) + ",";
    response_data += "\"processing_time_ms\":" + std::to_string(result.processing_time_ms) + ",";
    response_data += "\"avg_quality_score\":" + std::to_string(result.avg_quality_score) + ",";
    response_data += "\"avg_information_density\":" + std::to_string(result.avg_information_density) + ",";
    response_data += "\"high_quality_chunks\":" + std::to_string(result.high_quality_chunks) + ",";
    response_data += "\"total_title_tokens\":" + std::to_string(result.total_title_tokens) + ",";
    response_data += "\"total_metadata_tokens\":" + std::to_string(result.total_metadata_tokens) + ",";
    response_data += "\"total_content_tokens\":" + std::to_string(result.total_content_tokens) + ",";
    response_data += "\"total_rag_tokens\":" + std::to_string(result.total_rag_tokens) + ",";
    response_data += "\"chunks\":[";
    
    bool first_chunk = true;
    for (const auto& chunk : result.chunks) {
        if (!first_chunk) response_data += ",";
        response_data += "{";
        response_data += "\"chunk_id\":" + std::to_string(chunk.chunk_id) + ",";
        response_data += "\"content\":\"" + json_utils::escape_json_string(chunk.content) + "\",";
        response_data += "\"blurb\":\"" + json_utils::escape_json_string(chunk.blurb) + "\",";
        response_data += "\"title_prefix\":\"" + json_utils::escape_json_string(chunk.title_prefix) + "\",";
        response_data += "\"metadata_suffix_semantic\":\"" + json_utils::escape_json_string(chunk.metadata_suffix_semantic) + "\",";
        response_data += "\"metadata_suffix_keyword\":\"" + json_utils::escape_json_string(chunk.metadata_suffix_keyword) + "\",";
        response_data += "\"quality_score\":" + std::to_string(chunk.quality_score) + ",";
        response_data += "\"information_density\":" + std::to_string(chunk.information_density) + ",";
        response_data += "\"is_high_quality\":" + std::string(chunk.is_high_quality ? "true" : "false") + ",";
        response_data += "\"title_tokens\":" + std::to_string(chunk.title_tokens) + ",";
        response_data += "\"metadata_tokens\":" + std::to_string(chunk.metadata_tokens) + ",";
        response_data += "\"content_token_limit\":" + std::to_string(chunk.content_token_limit) + ",";
        response_data += "\"document_id\":\"" + json_utils::escape_json_string(chunk.document_id) + "\",";
        response_data += "\"source_type\":\"" + json_utils::escape_json_string(chunk.source_type) + "\",";
        response_data += "\"semantic_identifier\":\"" + json_utils::escape_json_string(chunk.semantic_identifier) + "\"}";
        first_chunk = false;
    }
    response_data += "]}";
    
    return response_data;
}

std::string serialize_system_info(int port, const std::string& host, int threads, 
                                 const std::string& upload_dir, int max_file_size_mb) {
    // Note: This would need access to processor_ to get statistics
    // For now, return a basic system info structure
    std::string response_data = "{\"server\":\"R3M Document Processing API\",";
    response_data += "\"version\":\"1.0.0\",";
    response_data += "\"port\":" + std::to_string(port) + ",";
    response_data += "\"host\":\"" + host + "\",";
    response_data += "\"threads\":" + std::to_string(threads) + ",";
    response_data += "\"upload_dir\":\"" + upload_dir + "\",";
    response_data += "\"max_file_size_mb\":" + std::to_string(max_file_size_mb) + "}";
    
    return response_data;
}

std::string serialize_performance_metrics(const core::ProcessingStats& stats) {
    std::string response_data = "{\"total_files_processed\":" + std::to_string(stats.total_files_processed) + ",";
    response_data += "\"successful_processing\":" + std::to_string(stats.successful_processing) + ",";
    response_data += "\"failed_processing\":" + std::to_string(stats.failed_processing) + ",";
    response_data += "\"filtered_out\":" + std::to_string(stats.filtered_out) + ",";
    response_data += "\"avg_processing_time_ms\":" + std::to_string(stats.avg_processing_time_ms) + ",";
    response_data += "\"total_text_extracted\":" + std::to_string(stats.total_text_extracted) + ",";
    response_data += "\"avg_content_quality_score\":" + std::to_string(stats.avg_content_quality_score);
    response_data += "}";
    return response_data;
}

} // namespace serialization
} // namespace api
} // namespace r3m 