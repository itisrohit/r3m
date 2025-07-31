cmake_minimum_required(VERSION 3.16)
project(r3m VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build directory: ${CMAKE_BINARY_DIR}")

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

# Find Crow HTTP server
find_package(Crow QUIET)
if(NOT Crow_FOUND)
    # Try to find Crow via pkg-config
    pkg_check_modules(CROW QUIET crow)
    if(CROW_FOUND)
        set(Crow_INCLUDE_DIRS ${CROW_INCLUDE_DIRS})
        set(Crow_LIBRARIES ${CROW_LIBRARIES})
        set(Crow_FOUND TRUE)
    endif()
endif()

# Manual Crow detection if pkg-config fails
if(NOT Crow_FOUND)
    find_path(Crow_INCLUDE_DIRS NAMES crow.h
        PATHS /opt/homebrew/Cellar/crow/*/include
        NO_DEFAULT_PATH
    )
    if(Crow_INCLUDE_DIRS)
        set(Crow_FOUND TRUE)
        set(Crow_LIBRARIES "")
        message(STATUS "Found Crow manually: ${Crow_INCLUDE_DIRS}")
    endif()
endif()

# Find ONNX Runtime (for future use)
find_package(ONNXRuntime QUIET)

# Find nlohmann_json (for JSON handling)
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    # Try to find via pkg-config
    pkg_check_modules(NLOHMANN_JSON QUIET nlohmann_json)
    if(NLOHMANN_JSON_FOUND)
        set(nlohmann_json_INCLUDE_DIRS ${NLOHMANN_JSON_INCLUDE_DIRS})
        set(nlohmann_json_LIBRARIES ${NLOHMANN_JSON_LIBRARIES})
        set(nlohmann_json_FOUND TRUE)
    endif()
endif()

# Set conditional compilation definitions
if(Crow_FOUND)
    add_definitions(-DR3M_HTTP_ENABLED)
    message(STATUS "HTTP server enabled (Crow found)")
else()
    message(STATUS "HTTP server disabled (Crow not found)")
endif()

# Only enable JSON if we can actually find the headers
if(nlohmann_json_FOUND)
    # Test if we can actually include the header
    include(CheckCXXSourceCompiles)
    set(CMAKE_REQUIRED_INCLUDES ${nlohmann_json_INCLUDE_DIRS})
    set(CMAKE_REQUIRED_LIBRARIES ${nlohmann_json_LIBRARIES})
    check_cxx_source_compiles("#include <nlohmann/json.hpp>\nint main() { return 0; }" NLOHMANN_JSON_WORKS)
    
    if(NLOHMANN_JSON_WORKS)
        add_definitions(-DR3M_JSON_ENABLED)
        message(STATUS "JSON library enabled (nlohmann_json found and working)")
    else()
        set(nlohmann_json_FOUND FALSE)
        message(STATUS "JSON library disabled (nlohmann_json headers not found)")
    endif()
else()
    message(STATUS "JSON library disabled (nlohmann_json not found)")
endif()

# Find poppler-cpp
pkg_check_modules(POPPLER_CPP REQUIRED poppler-cpp)
if(POPPLER_CPP_FOUND)
    message(STATUS "Found poppler-cpp: ${POPPLER_CPP_VERSION}")
    include_directories(${POPPLER_CPP_INCLUDE_DIRS})
    link_directories(${POPPLER_CPP_LIBRARY_DIRS})
endif()

# Find gumbo
pkg_check_modules(GUMBO REQUIRED gumbo)
if(GUMBO_FOUND)
    message(STATUS "Found gumbo: ${GUMBO_VERSION}")
    include_directories(${GUMBO_INCLUDE_DIRS})
    link_directories(${GUMBO_LIBRARY_DIRS})
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Cross-platform stdc++fs linking
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++fs")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++fs")
endif()

# Source files
set(CORE_SOURCES
    src/core/document_processor.cpp
    src/core/config_manager.cpp
    src/core/library.cpp
)

set(CHUNKING_SOURCES
    src/chunking/tokenizer.cpp
    src/chunking/sentence_chunker.cpp
    src/chunking/multipass_chunker.cpp
    src/chunking/contextual_rag.cpp
    src/chunking/advanced_chunker.cpp
    src/chunking/metadata_processor.cpp
    src/chunking/advanced_tokenizer.cpp
)

set(PROCESSING_SOURCES
    src/processing/pipeline.cpp
)

set(QUALITY_SOURCES
    src/quality/assessor.cpp
)

set(PARALLEL_SOURCES
    src/parallel/thread_pool.cpp
)

set(FORMATS_SOURCES
    src/formats/processor.cpp
)

set(UTILS_SOURCES
    src/utils/text_utils.cpp
    src/utils/text_processing.cpp
)

set(SERVER_SOURCES
    src/server/http_server.cpp
    src/api/routes/routes.cpp
    src/api/jobs/job_manager.cpp
    src/api/config/config.cpp
)

set(MAIN_SOURCES
    src/main.cpp
)

# Combine all sources
set(ALL_SOURCES
    ${CORE_SOURCES}
    ${CHUNKING_SOURCES}
    ${PROCESSING_SOURCES}
    ${QUALITY_SOURCES}
    ${PARALLEL_SOURCES}
    ${FORMATS_SOURCES}
    ${UTILS_SOURCES}
    ${SERVER_SOURCES}
    ${MAIN_SOURCES}
)

# Create main R3M executable
add_executable(r3m ${ALL_SOURCES})

# Set include directories
target_include_directories(r3m PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${POPPLER_CPP_INCLUDE_DIRS}
    ${GUMBO_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(r3m 
    ${POPPLER_CPP_LIBRARIES}
    ${GUMBO_LIBRARIES}
    Threads::Threads
)

# Add Crow HTTP server if found
if(Crow_FOUND)
    target_link_libraries(r3m Crow::Crow)
    target_compile_definitions(r3m PRIVATE R3M_HTTP_ENABLED)
endif()

# Add JSON library if found
if(nlohmann_json_FOUND)
    target_include_directories(r3m PRIVATE ${nlohmann_json_INCLUDE_DIRS})
    target_link_libraries(r3m ${nlohmann_json_LIBRARIES})
    target_compile_definitions(r3m PRIVATE R3M_JSON_ENABLED)
endif()

# Cross-platform compatibility
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(r3m stdc++fs)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(r3m stdc++fs)
endif()

# Main comprehensive test executable
add_executable(r3m-test
    tests/test_comprehensive.cpp
    ${CORE_SOURCES}
    ${CHUNKING_SOURCES}
    ${PROCESSING_SOURCES}
    ${QUALITY_SOURCES}
    ${PARALLEL_SOURCES}
    ${FORMATS_SOURCES}
    ${UTILS_SOURCES}
    ${SERVER_SOURCES}
)

# HTTP server test executable
add_executable(r3m-http-test
    tests/test_http_server.cpp
    ${CORE_SOURCES}
    ${CHUNKING_SOURCES}
    ${PROCESSING_SOURCES}
    ${QUALITY_SOURCES}
    ${PARALLEL_SOURCES}
    ${FORMATS_SOURCES}
    ${UTILS_SOURCES}
    ${SERVER_SOURCES}
)

# Comprehensive chunking test executable
add_executable(r3m-chunking-test
    tests/test_chunking_comprehensive.cpp
    ${CORE_SOURCES}
    ${CHUNKING_SOURCES}
    ${PROCESSING_SOURCES}
    ${QUALITY_SOURCES}
    ${PARALLEL_SOURCES}
    ${FORMATS_SOURCES}
    ${UTILS_SOURCES}
)

# Link libraries for the main executable
target_link_libraries(r3m-test
    ${CMAKE_THREAD_LIBS_INIT}
    ${POPPLER_CPP_LIBRARIES}
    ${GUMBO_LIBRARIES}
)

# Add Crow to test executables if found
if(Crow_FOUND)
    target_link_libraries(r3m-test Crow::Crow)
    target_compile_definitions(r3m-test PRIVATE R3M_HTTP_ENABLED)
endif()

# Link libraries for the HTTP server test
target_link_libraries(r3m-http-test
    ${CMAKE_THREAD_LIBS_INIT}
    ${POPPLER_CPP_LIBRARIES}
    ${GUMBO_LIBRARIES}
)

# Add Crow to HTTP test executable if found
if(Crow_FOUND)
    target_link_libraries(r3m-http-test Crow::Crow)
    target_compile_definitions(r3m-http-test PRIVATE R3M_HTTP_ENABLED)
endif()

# Link libraries for the comprehensive chunking test
target_link_libraries(r3m-chunking-test
    ${CMAKE_THREAD_LIBS_INIT}
    ${POPPLER_CPP_LIBRARIES}
    ${GUMBO_LIBRARIES}
)

# Advanced features test executable
add_executable(r3m-advanced-features-test
    tests/test_advanced_features.cpp
    ${CORE_SOURCES}
    ${CHUNKING_SOURCES}
    ${PROCESSING_SOURCES}
    ${QUALITY_SOURCES}
    ${PARALLEL_SOURCES}
    ${FORMATS_SOURCES}
    ${UTILS_SOURCES}
)

# Link libraries for the advanced features test
target_link_libraries(r3m-advanced-features-test
    ${CMAKE_THREAD_LIBS_INIT}
    ${POPPLER_CPP_LIBRARIES}
    ${GUMBO_LIBRARIES}
)

# Document processor integration test
add_executable(r3m-document-processor-integration-test tests/test_document_processor_integration.cpp)
target_include_directories(r3m-document-processor-integration-test PRIVATE include)
target_sources(r3m-document-processor-integration-test PRIVATE 
    ${CORE_SOURCES}
    ${CHUNKING_SOURCES}
    ${PROCESSING_SOURCES}
    ${QUALITY_SOURCES}
    ${PARALLEL_SOURCES}
    ${FORMATS_SOURCES}
    ${UTILS_SOURCES}
)
target_link_libraries(r3m-document-processor-integration-test ${CMAKE_THREAD_LIBS_INIT} ${POPPLER_CPP_LIBRARIES} ${GUMBO_LIBRARIES})

# Library example
add_executable(r3m-library-example examples/direct_library_usage.cpp)
target_include_directories(r3m-library-example PRIVATE include)
target_sources(r3m-library-example PRIVATE 
    ${CORE_SOURCES}
    ${CHUNKING_SOURCES}
    ${PROCESSING_SOURCES}
    ${QUALITY_SOURCES}
    ${PARALLEL_SOURCES}
    ${FORMATS_SOURCES}
    ${UTILS_SOURCES}
)
target_link_libraries(r3m-library-example ${CMAKE_THREAD_LIBS_INIT} ${POPPLER_CPP_LIBRARIES} ${GUMBO_LIBRARIES})

# Large document chunking test
add_executable(r3m-large-document-test tests/test_large_document_chunking.cpp)
target_include_directories(r3m-large-document-test PRIVATE include)
target_sources(r3m-large-document-test PRIVATE 
    ${CORE_SOURCES}
    ${CHUNKING_SOURCES}
    ${PROCESSING_SOURCES}
    ${QUALITY_SOURCES}
    ${PARALLEL_SOURCES}
    ${FORMATS_SOURCES}
    ${UTILS_SOURCES}
)
target_link_libraries(r3m-large-document-test ${CMAKE_THREAD_LIBS_INIT} ${POPPLER_CPP_LIBRARIES} ${GUMBO_LIBRARIES})

# Custom targets for build management
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.txt
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.md
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.json
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.csv
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.log
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.html
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.pdf
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.xml
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.yml
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.yaml
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.tsv
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.conf
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.mdx
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/large_file_*.txt
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/empty.txt
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/low_quality.txt
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/technical_document.txt
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/sample.txt
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/readme.md
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/config.json
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/test.html
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/*.o
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/*.a
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/*.so
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/*.dylib
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/Makefile
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/r3m-test
    COMMENT "Cleaning all build artifacts and generated test files (keeping results/reports)"
)

add_custom_target(clean-test-data
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.txt
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.md
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.json
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.csv
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.log
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.html
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.pdf
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.xml
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.yml
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.yaml
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.tsv
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.conf
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/*.mdx
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/large_file_*.txt
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/empty.txt
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/low_quality.txt
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/technical_document.txt
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/sample.txt
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/readme.md
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/config.json
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/data/test.html
    COMMENT "Cleaning only generated test files (keeping results/reports)"
)

add_custom_target(clean-objs
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/*.o
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/*.a
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/*.so
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/*.dylib
    COMMENT "Cleaning object files and libraries"
)

add_custom_target(rebuild
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
    COMMENT "Rebuilding from scratch"
)

add_custom_target(test
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR} ./r3m-test
    COMMENT "Building and running comprehensive test"
)

# Print module information
message(STATUS "Core sources: ${CORE_SOURCES}")
message(STATUS "Processing sources: ${PROCESSING_SOURCES}")
message(STATUS "Quality sources: ${QUALITY_SOURCES}")
message(STATUS "Parallel sources: ${PARALLEL_SOURCES}")
message(STATUS "Formats sources: ${FORMATS_SOURCES}")
message(STATUS "Utils sources: ${UTILS_SOURCES}")
message(STATUS "Total sources: ${ALL_SOURCES}") 