cmake_minimum_required(VERSION 3.20)
project(R3M VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Manual include paths for poppler and gumbo
include_directories(/opt/homebrew/Cellar/poppler/25.07.0_1/include/poppler/cpp)
include_directories(/opt/homebrew/Cellar/poppler/25.07.0_1/include/poppler)
include_directories(/opt/homebrew/include)

# Source files
set(CORE_SOURCES
    src/core/document_processor.cpp
    src/core/config_manager.cpp
)

# Main executable (comprehensive test)
add_executable(r3m-test
    tests/test_output_to_data.cpp
    ${CORE_SOURCES}
)

# Cross-platform filesystem support
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    # Older GCC versions need stdc++fs
    target_link_libraries(r3m-test stdc++fs)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Clang on Linux might need stdc++fs
    target_link_libraries(r3m-test stdc++fs)
endif()

# Manual library paths and linking
target_link_libraries(r3m-test
    /opt/homebrew/Cellar/poppler/25.07.0_1/lib/libpoppler-cpp.dylib
    /opt/homebrew/lib/libgumbo.dylib
)

# Enhanced clean targets
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/Makefile
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/compile_commands.json
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/Testing
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CTestTestfile.cmake
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/DartConfiguration.tcl
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/r3m-test
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/pdf-test
    COMMENT "Cleaning all build files and artifacts"
)

# Clean object files only
add_custom_target(clean-objs
    COMMAND find ${CMAKE_BINARY_DIR} -name "*.o" -delete
    COMMAND find ${CMAKE_BINARY_DIR} -name "*.a" -delete
    COMMAND find ${CMAKE_BINARY_DIR} -name "*.so" -delete
    COMMAND find ${CMAKE_BINARY_DIR} -name "*.dylib" -delete
    COMMENT "Cleaning object files only"
)

# Rebuild target
add_custom_target(rebuild
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
    COMMENT "Clean rebuild"
)

# Test target
add_executable(test-output-to-data
    tests/test_output_to_data.cpp
    ${CORE_SOURCES}
)

target_link_libraries(test-output-to-data
    /opt/homebrew/Cellar/poppler/25.07.0_1/lib/libpoppler-cpp.dylib
    /opt/homebrew/lib/libgumbo.dylib
)

add_custom_target(run-output-to-data
    COMMAND ${CMAKE_BINARY_DIR}/test-output-to-data
    DEPENDS test-output-to-data
    COMMENT "Running test-output-to-data"
)

add_custom_target(test-all
    COMMAND ${CMAKE_BINARY_DIR}/r3m-test
    COMMAND ${CMAKE_BINARY_DIR}/test-output-to-data
    DEPENDS r3m-test test-output-to-data
    COMMENT "Running all tests"
)

# Install
install(TARGETS r3m-test test-output-to-data DESTINATION bin)

# Print build info
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build directory: ${CMAKE_BINARY_DIR}") 