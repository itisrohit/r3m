#include "r3m/api/routes/routes.hpp"
#include "r3m/utils/text_utils.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>
#include <iostream>

// JSON fallback when nlohmann/json is not available
#ifndef R3M_JSON_ENABLED
namespace json {
    class basic_json {
    public:
        std::string dump() const { return "{}"; }
        void push_back(const basic_json&) {}
        basic_json& operator[](const std::string&) { return *this; }
        basic_json& operator[](int) { return *this; }
        template<typename T>
        basic_json& operator=(const T&) { return *this; }
        bool has(const std::string&) const { return false; }
        std::string s() const { return ""; }
        bool b() const { return false; }
        int i() const { return 0; }
        double d() const { return 0.0; }
    };
    using json = basic_json;
    json array() { return json(); }
    json parse(const std::string&) { return json(); }
}
#endif

namespace r3m {
namespace api {

Routes::Routes(std::shared_ptr<core::DocumentProcessor> processor)
    : processor_(processor) {
}

#ifdef R3M_HTTP_ENABLED

crow::response Routes::handle_health_check() {
    crow::response res;
    res.code = 200;
    res.set_header("Content-Type", "application/json");
    res.body = create_response(true, "R3M server is healthy");
    return res;
}

crow::response Routes::handle_process_document(const crow::request& req) {
    crow::response res;
    res.set_header("Content-Type", "application/json");
    
    try {
        // Parse request body
        auto body = crow::json::load(req.body);
        if (!body) {
            res.code = 400;
            res.body = create_response(false, "Invalid JSON request");
            return res;
        }
        
        // Extract file path or content
        std::string file_path;
        if (body.has("file_path")) {
            file_path = body["file_path"].s();
        } else if (body.has("file_content")) {
            // Save uploaded content to temporary file
            std::string filename = "upload_" + generate_job_id() + ".txt";
            file_path = "/tmp/r3m/uploads/" + filename;
            
            // Create upload directory if it doesn't exist
            std::filesystem::create_directories("/tmp/r3m/uploads");
            
            std::ofstream file(file_path);
            if (!file.is_open()) {
                res.code = 500;
                res.body = create_response(false, "Failed to save uploaded file");
                return res;
            }
            file << body["file_content"].s();
            file.close();
        } else {
            res.code = 400;
            res.body = create_response(false, "Missing file_path or file_content");
            return res;
        }
        
        // Process the document
        std::cout << "Processing file: " << file_path << std::endl;
        auto result = processor_->process_document(file_path);
        
        // Create response with chunking information
        std::string response_data = serialize_document_result_with_chunks(result);
        
        res.code = 200;
        res.body = create_response(true, "Document processed successfully", response_data);
        
    } catch (const std::exception& e) {
        res.code = 500;
        res.body = create_response(false, "Processing error: " + std::string(e.what()));
    }
    
    return res;
}

crow::response Routes::handle_process_batch(const crow::request& req) {
    crow::response res;
    res.set_header("Content-Type", "application/json");
    
    try {
        // Parse request body
        auto body = crow::json::load(req.body);
        if (!body || !body.has("files")) {
            res.code = 400;
            res.body = create_response(false, "Invalid request: missing files array");
            return res;
        }
        
        std::vector<std::string> file_paths;
        for (const auto& file : body["files"]) {
            file_paths.push_back(file.s());
        }
        
        // Process batch
        auto results = processor_->process_documents_parallel(file_paths);
        
        // Create response with chunking information
        std::string response_data = serialize_batch_results_with_chunks(results);
        
        res.code = 200;
        res.body = create_response(true, "Batch processing completed", response_data);
        
    } catch (const std::exception& e) {
        res.code = 500;
        res.body = create_response(false, "Batch processing error: " + std::string(e.what()));
    }
    
    return res;
}

crow::response Routes::handle_chunk_document(const crow::request& req) {
    crow::response res;
    res.set_header("Content-Type", "application/json");
    
    try {
        // Parse request body
        auto body = crow::json::load(req.body);
        if (!body) {
            res.code = 400;
            res.body = create_response(false, "Invalid JSON request");
            return res;
        }
        
        // Extract file path
        std::string file_path;
        if (body.has("file_path")) {
            file_path = body["file_path"].s();
        } else {
            res.code = 400;
            res.body = create_response(false, "Missing file_path");
            return res;
        }
        
        // Process document with chunking
        std::cout << "Chunking file: " << file_path << std::endl;
        auto chunking_result = processor_->process_document_with_chunking(file_path);
        
        // Create response with chunking results
        std::string response_data = serialize_chunking_result(chunking_result);
        
        res.code = 200;
        res.body = create_response(true, "Document chunking completed", response_data);
        
    } catch (const std::exception& e) {
        res.code = 500;
        res.body = create_response(false, "Chunking error: " + std::string(e.what()));
    }
    
    return res;
}

crow::response Routes::handle_job_status(const std::string& job_id) {
    (void)job_id; // Suppress unused parameter warning
    crow::response res;
    res.set_header("Content-Type", "application/json");
    
    // For now, return job not found (async jobs not implemented yet)
    res.code = 404;
    res.body = create_response(false, "Job not found");
    return res;
}

crow::response Routes::handle_system_info() {
    crow::response res;
    res.set_header("Content-Type", "application/json");
    
    std::string response_data = serialize_system_info();
    
    res.code = 200;
    res.body = create_response(true, "System information retrieved", response_data);
    return res;
}

#endif

std::string Routes::create_response(bool success, const std::string& message, const std::string& data) {
#ifdef R3M_JSON_ENABLED
    json response;
    response["success"] = success;
    response["message"] = message;
    if (!data.empty()) {
        response["data"] = json::parse(data);
    }
    return response.dump();
#else
    // Simple text response if JSON not available
    if (data.empty()) {
        return "{\"success\":" + std::string(success ? "true" : "false") + 
               ",\"message\":\"" + message + "\"}";
    } else {
        return "{\"success\":" + std::string(success ? "true" : "false") + 
               ",\"message\":\"" + message + "\",\"data\":" + data + "}";
    }
#endif
}

std::string Routes::generate_job_id() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char* hex_chars = "0123456789abcdef";
    
    std::string job_id;
    for (int i = 0; i < 32; ++i) {
        job_id += hex_chars[dis(gen)];
    }
    return job_id;
}

std::string Routes::serialize_document_result(const core::DocumentResult& result) {
    std::string response_data = "{\"job_id\":\"" + generate_job_id() + "\",";
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

std::string Routes::serialize_document_result_with_chunks(const core::DocumentResult& result) {
    std::string response_data = "{\"job_id\":\"" + generate_job_id() + "\",";
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
        response_data += "\"content\":\"" + chunk.content + "\",";
        response_data += "\"blurb\":\"" + chunk.blurb + "\",";
        response_data += "\"title_prefix\":\"" + chunk.title_prefix + "\",";
        response_data += "\"metadata_suffix_semantic\":\"" + chunk.metadata_suffix_semantic + "\",";
        response_data += "\"metadata_suffix_keyword\":\"" + chunk.metadata_suffix_keyword + "\",";
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

std::string Routes::serialize_batch_results(const std::vector<core::DocumentResult>& results) {
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

std::string Routes::serialize_batch_results_with_chunks(const std::vector<core::DocumentResult>& results) {
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
            response_data += "\"content\":\"" + chunk.content + "\",";
            response_data += "\"blurb\":\"" + chunk.blurb + "\",";
            response_data += "\"title_prefix\":\"" + chunk.title_prefix + "\",";
            response_data += "\"metadata_suffix_semantic\":\"" + chunk.metadata_suffix_semantic + "\",";
            response_data += "\"metadata_suffix_keyword\":\"" + chunk.metadata_suffix_keyword + "\",";
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

std::string Routes::serialize_chunking_result(const chunking::ChunkingResult& result) {
    std::string response_data = "{\"job_id\":\"" + generate_job_id() + "\",";
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
        response_data += "\"content\":\"" + chunk.content + "\",";
        response_data += "\"blurb\":\"" + chunk.blurb + "\",";
        response_data += "\"title_prefix\":\"" + chunk.title_prefix + "\",";
        response_data += "\"metadata_suffix_semantic\":\"" + chunk.metadata_suffix_semantic + "\",";
        response_data += "\"metadata_suffix_keyword\":\"" + chunk.metadata_suffix_keyword + "\",";
        response_data += "\"quality_score\":" + std::to_string(chunk.quality_score) + ",";
        response_data += "\"information_density\":" + std::to_string(chunk.information_density) + ",";
        response_data += "\"is_high_quality\":" + std::string(chunk.is_high_quality ? "true" : "false") + ",";
        response_data += "\"title_tokens\":" + std::to_string(chunk.title_tokens) + ",";
        response_data += "\"metadata_tokens\":" + std::to_string(chunk.metadata_tokens) + ",";
        response_data += "\"content_token_limit\":" + std::to_string(chunk.content_token_limit) + ",";
        response_data += "\"document_id\":\"" + chunk.document_id + "\",";
        response_data += "\"source_type\":\"" + chunk.source_type + "\",";
        response_data += "\"semantic_identifier\":\"" + chunk.semantic_identifier + "\"}";
        first_chunk = false;
    }
    response_data += "]}";
    
    return response_data;
}

std::string Routes::serialize_system_info() {
    auto stats = processor_->get_statistics();
    
    std::string response_data = "{\"server\":\"R3M Document Processing API\",";
    response_data += "\"version\":\"1.0.0\",";
    response_data += "\"port\":8080,";
    response_data += "\"host\":\"0.0.0.0\",";
    response_data += "\"threads\":4,";
    response_data += "\"upload_dir\":\"/tmp/r3m/uploads\",";
    response_data += "\"max_file_size_mb\":100,";
    response_data += "\"statistics\":{";
    response_data += "\"total_files_processed\":" + std::to_string(stats.total_files_processed) + ",";
    response_data += "\"successful_processing\":" + std::to_string(stats.successful_processing) + ",";
    response_data += "\"failed_processing\":" + std::to_string(stats.failed_processing) + ",";
    response_data += "\"filtered_out\":" + std::to_string(stats.filtered_out) + ",";
    response_data += "\"avg_processing_time_ms\":" + std::to_string(stats.avg_processing_time_ms) + ",";
    response_data += "\"total_text_extracted\":" + std::to_string(stats.total_text_extracted) + ",";
    response_data += "\"avg_content_quality_score\":" + std::to_string(stats.avg_content_quality_score);
    response_data += "}}";
    
    return response_data;
}

} // namespace api
} // namespace r3m 