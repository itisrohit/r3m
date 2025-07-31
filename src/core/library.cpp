#include "r3m/core/library.hpp"
#include <iostream>
#include <chrono>

namespace r3m {
namespace core {

Library::Library() = default;

bool Library::initialize(const std::string& config_path) {
    try {
        // Initialize config manager
        config_manager_ = std::make_unique<ConfigManager>();
        if (!config_manager_->load_config(config_path)) {
            std::cerr << "Failed to load configuration from: " << config_path << std::endl;
            return false;
        }
        
        // Get configuration
        auto config = config_manager_->get_all_config();
        
        // Initialize document processor
        processor_ = std::make_unique<DocumentProcessor>();
        if (!processor_->initialize(config)) {
            std::cerr << "Failed to initialize document processor" << std::endl;
            return false;
        }
        
        initialized_ = true;
        std::cout << "✅ R3M Library initialized successfully" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize R3M Library: " << e.what() << std::endl;
        return false;
    }
}

bool Library::initialize(const std::unordered_map<std::string, std::string>& config) {
    try {
        // Initialize document processor directly with config
        processor_ = std::make_unique<DocumentProcessor>();
        if (!processor_->initialize(config)) {
            std::cerr << "Failed to initialize document processor" << std::endl;
            return false;
        }
        
        initialized_ = true;
        std::cout << "✅ R3M Library initialized successfully" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize R3M Library: " << e.what() << std::endl;
        return false;
    }
}

DocumentResult Library::process_document(const std::string& file_path) {
    if (!initialized_) {
        throw std::runtime_error("R3M Library not initialized");
    }
    
    return processor_->process_document(file_path);
}

std::vector<DocumentResult> Library::process_documents_parallel(const std::vector<std::string>& file_paths) {
    if (!initialized_) {
        throw std::runtime_error("R3M Library not initialized");
    }
    
    return processor_->process_documents_parallel(file_paths);
}

std::vector<DocumentResult> Library::process_documents_batch(const std::vector<std::string>& file_paths) {
    if (!initialized_) {
        throw std::runtime_error("R3M Library not initialized");
    }
    
    return processor_->process_documents_batch(file_paths);
}

Library::BatchResult Library::process_batch_with_filtering(const std::vector<std::string>& file_paths) {
    if (!initialized_) {
        throw std::runtime_error("R3M Library not initialized");
    }
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Process documents in parallel
    auto results = processor_->process_documents_parallel(file_paths);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    BatchResult batch_result;
    batch_result.total_files = file_paths.size();
    batch_result.processing_time_ms = duration.count() / 1000.0;
    
    // Separate processed and filtered results
    for (const auto& result : results) {
        if (result.processing_success) {
            if (processor_->should_filter_document(result)) {
                batch_result.filtered_out.push_back(result);
            } else {
                batch_result.processed.push_back(result);
            }
            batch_result.successful_processing++;
        } else {
            batch_result.failed_processing++;
        }
    }
    
    return batch_result;
}

ProcessingStats Library::get_statistics() const {
    if (!initialized_) {
        throw std::runtime_error("R3M Library not initialized");
    }
    
    return processor_->get_statistics();
}

void Library::update_config(const std::unordered_map<std::string, std::string>& config) {
    if (!initialized_) {
        throw std::runtime_error("R3M Library not initialized");
    }
    
    // Reinitialize processor with new config
    processor_->initialize(config);
}

std::unordered_map<std::string, std::string> Library::get_config() const {
    if (!config_manager_) {
        return {};
    }
    
    return config_manager_->get_all_config();
}

} // namespace core
} // namespace r3m 