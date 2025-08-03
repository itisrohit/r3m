#pragma once

#include "r3m/core/document_processor.hpp"
#include "r3m/chunking/chunk_models.hpp"
#include <vector>
#include <string>

namespace r3m {
namespace api {
namespace serialization {

/**
 * @brief Serialize a single document result to JSON
 * @param result Document processing result
 * @return JSON string representation
 */
std::string serialize_document_result(const core::DocumentResult& result);

/**
 * @brief Serialize a document result with chunking information
 * @param result Document processing result with chunks
 * @return JSON string representation with chunks
 */
std::string serialize_document_result_with_chunks(const core::DocumentResult& result);

/**
 * @brief Serialize batch processing results
 * @param results Vector of document processing results
 * @return JSON string representation
 */
std::string serialize_batch_results(const std::vector<core::DocumentResult>& results);

/**
 * @brief Serialize batch processing results with chunking information
 * @param results Vector of document processing results with chunks
 * @return JSON string representation with chunks
 */
std::string serialize_batch_results_with_chunks(const std::vector<core::DocumentResult>& results);

/**
 * @brief Serialize chunking result
 * @param result Chunking processing result
 * @return JSON string representation
 */
std::string serialize_chunking_result(const chunking::ChunkingResult& result);

/**
 * @brief Serialize system information
 * @return JSON string representation of system info
 */
std::string serialize_system_info();

/**
 * @brief Serialize performance metrics
 * @param stats Processing statistics
 * @return JSON string representation
 */
std::string serialize_performance_metrics(const core::ProcessingStats& stats);

} // namespace serialization
} // namespace api
} // namespace r3m 