#include "r3m/api/routes/route_handlers/route_handlers.hpp"
#include "r3m/api/routes/response_handler/response_handler.hpp"
#include "r3m/api/routes/serialization/serializer.hpp"
#include "r3m/utils/text_utils.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

#ifdef R3M_HTTP_ENABLED

namespace r3m {
namespace api {
namespace route_handlers {

crow::response handle_health_check() {
    crow::response res;
    res.code = 200;
    res.set_header("Content-Type", "application/json");
    res.body = response_handler::create_response(true, "R3M server is healthy");
    return res;
}

crow::response handle_process_document(const crow::request& req, std::shared_ptr<core::DocumentProcessor> processor) {
    crow::response res;
    res.set_header("Content-Type", "application/json");
    
    try {
        // Parse request body
        auto body = crow::json::load(req.body);
        if (!body) {
            res.code = 400;
            res.body = response_handler::create_response(false, "Invalid JSON request");
            return res;
        }
        
        // Extract file path or content
        std::string file_path;
        if (body.has("file_path")) {
            file_path = body["file_path"].s();
        } else if (body.has("file_content")) {
            // Save uploaded content to data directory
            std::string filename = "upload_" + response_handler::generate_job_id() + ".txt";
            file_path = "data/" + filename;
            
            // Create data directory if it doesn't exist
            std::filesystem::create_directories("data");
            
            std::ofstream file(file_path);
            if (!file.is_open()) {
                res.code = 500;
                res.body = response_handler::create_response(false, "Failed to save uploaded file");
                return res;
            }
            file << body["file_content"].s();
            file.close();
        } else {
            res.code = 400;
            res.body = response_handler::create_response(false, "Missing file_path or file_content");
            return res;
        }
        
        // Process the document
        std::cout << "Processing file: " << file_path << std::endl;
        auto result = processor->process_document(file_path);
        
        // Create response with chunking information
        std::string response_data = serialization::serialize_document_result_with_chunks(result);
        
        res.code = 200;
        res.body = response_handler::create_response(true, "Document processed successfully", response_data);
        
    } catch (const std::exception& e) {
        res.code = 500;
        res.body = response_handler::create_response(false, "Processing error: " + std::string(e.what()));
    }
    
    return res;
}

crow::response handle_process_batch(const crow::request& req, std::shared_ptr<core::DocumentProcessor> processor) {
    crow::response res;
    res.set_header("Content-Type", "application/json");
    
    try {
        // Parse request body
        auto body = crow::json::load(req.body);
        if (!body || !body.has("files")) {
            res.code = 400;
            res.body = response_handler::create_response(false, "Invalid request: missing files array");
            return res;
        }
        
        std::vector<std::string> file_paths;
        for (const auto& file : body["files"]) {
            file_paths.push_back(file.s());
        }
        
        // Process batch
        auto results = processor->process_documents_parallel(file_paths);
        
        // Create response with chunking information
        std::string response_data = serialization::serialize_batch_results_with_chunks(results);
        
        res.code = 200;
        res.body = response_handler::create_response(true, "Batch processing completed", response_data);
        
    } catch (const std::exception& e) {
        res.code = 500;
        res.body = response_handler::create_response(false, "Batch processing error: " + std::string(e.what()));
    }
    
    return res;
}

crow::response handle_chunk_document(const crow::request& req, std::shared_ptr<core::DocumentProcessor> processor) {
    crow::response res;
    res.set_header("Content-Type", "application/json");
    
    try {
        // Parse request body
        auto body = crow::json::load(req.body);
        if (!body) {
            res.code = 400;
            res.body = response_handler::create_response(false, "Invalid JSON request");
            return res;
        }
        
        // Extract file path
        std::string file_path;
        if (body.has("file_path")) {
            file_path = body["file_path"].s();
        } else {
            res.code = 400;
            res.body = response_handler::create_response(false, "Missing file_path");
            return res;
        }
        
        // Process document with chunking
        std::cout << "Chunking file: " << file_path << std::endl;
        auto chunking_result = processor->process_document_with_chunking(file_path);
        
        // Create response with chunking results
        std::string response_data = serialization::serialize_chunking_result(chunking_result);
        
        res.code = 200;
        res.body = response_handler::create_response(true, "Document chunking completed", response_data);
        
    } catch (const std::exception& e) {
        res.code = 500;
        res.body = response_handler::create_response(false, "Chunking error: " + std::string(e.what()));
    }
    
    return res;
}

crow::response handle_job_status(const std::string& job_id) {
    (void)job_id; // Suppress unused parameter warning
    crow::response res;
    res.set_header("Content-Type", "application/json");
    
    // For now, return job not found (async jobs not implemented yet)
    res.code = 404;
    res.body = response_handler::create_response(false, "Job not found");
    return res;
}

crow::response handle_system_info(std::shared_ptr<core::DocumentProcessor> processor, 
                                 const std::unordered_map<std::string, std::string>& config) {
    (void)processor; // Suppress unused parameter warning
    crow::response res;
    res.set_header("Content-Type", "application/json");
    
    // Extract configuration values with defaults
    int port = 7860;
    std::string host = "0.0.0.0";
    int threads = 4;
    std::string upload_dir = "/tmp/r3m/uploads";
    int max_file_size_mb = 100;
    
    if (config.count("server.port")) {
        port = std::stoi(config.at("server.port"));
    }
    if (config.count("server.host")) {
        host = config.at("server.host");
    }
    if (config.count("server.threads")) {
        threads = std::stoi(config.at("server.threads"));
    }
    if (config.count("server.upload_dir")) {
        upload_dir = config.at("server.upload_dir");
    }
    if (config.count("server.max_file_size_mb")) {
        max_file_size_mb = std::stoi(config.at("server.max_file_size_mb"));
    }
    
    std::string response_data = serialization::serialize_system_info(port, host, threads, upload_dir, max_file_size_mb);
    
    res.code = 200;
    res.body = response_handler::create_response(true, "System information retrieved", response_data);
    return res;
}

crow::response handle_metrics(std::shared_ptr<core::DocumentProcessor> processor) {
    crow::response res;
    res.set_header("Content-Type", "application/json");
    
    try {
        // Get performance metrics from processor
        auto stats = processor->get_statistics();
        
        std::string response_data = serialization::serialize_performance_metrics(stats);
        
        res.code = 200;
        res.body = response_handler::create_response(true, "Performance metrics retrieved", response_data);
        
    } catch (const std::exception& e) {
        res.code = 500;
        res.body = response_handler::create_response(false, "Error retrieving metrics: " + std::string(e.what()));
    }
    
    return res;
}

} // namespace route_handlers
} // namespace api
} // namespace r3m

#endif 