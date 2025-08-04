#pragma once

#include "r3m/core/document_processor.hpp"
#include <memory>
#include <string>

#ifdef R3M_HTTP_ENABLED
#include <crow.h>

namespace r3m {
namespace api {
namespace route_handlers {

/**
 * @brief Handle health check endpoint
 * @return Crow response with health status
 */
crow::response handle_health_check();

/**
 * @brief Handle single document processing endpoint
 * @param req Crow request object
 * @param processor Document processor instance
 * @return Crow response with processing results
 */
crow::response handle_process_document(const crow::request& req, std::shared_ptr<core::DocumentProcessor> processor);

/**
 * @brief Handle batch document processing endpoint
 * @param req Crow request object
 * @param processor Document processor instance
 * @return Crow response with batch processing results
 */
crow::response handle_process_batch(const crow::request& req, std::shared_ptr<core::DocumentProcessor> processor);

/**
 * @brief Handle document chunking endpoint
 * @param req Crow request object
 * @param processor Document processor instance
 * @return Crow response with chunking results
 */
crow::response handle_chunk_document(const crow::request& req, std::shared_ptr<core::DocumentProcessor> processor);

/**
 * @brief Handle job status endpoint
 * @param job_id Job identifier
 * @return Crow response with job status
 */
crow::response handle_job_status(const std::string& job_id);

/**
 * @brief Handle system information endpoint
 * @param processor Document processor instance
 * @return Crow response with system information
 */
crow::response handle_system_info(std::shared_ptr<core::DocumentProcessor> processor, 
                                 const std::unordered_map<std::string, std::string>& config);

/**
 * @brief Handle performance metrics endpoint
 * @param processor Document processor instance
 * @return Crow response with performance metrics
 */
crow::response handle_metrics(std::shared_ptr<core::DocumentProcessor> processor);

} // namespace route_handlers
} // namespace api
} // namespace r3m
#endif 