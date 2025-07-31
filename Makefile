# R3M Build System
# ================

# Variables
BUILD_DIR = build
CMAKE = cmake
MAKE = make

# Default target
all: build

# Build the project
build:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(CMAKE) ..
	cd $(BUILD_DIR) && $(MAKE)

# Clean build artifacts
clean:
	cd $(BUILD_DIR) && $(MAKE) clean-all

# Clean everything (including CMake cache)
clean-all:
	rm -rf $(BUILD_DIR)
	./scripts/clean_test_data.sh
	rm -f *.o *.a *.so *.dylib
	find . -name "*.o" -delete
	find . -name "*.a" -delete
	find . -name "*.so" -delete
	find . -name "*.dylib" -delete

# Clean only test data (keep results/reports)
clean-test-data:
	./scripts/clean_test_data.sh

# Rebuild from scratch
rebuild: clean-all build

# Run tests
test: build
	cd $(BUILD_DIR) && $(MAKE) test

# Run main tests
test-main: build
	cd $(BUILD_DIR) && ./r3m-test

# Install
install: build
	cd $(BUILD_DIR) && $(MAKE) install

# Show build info
info:
	@echo "R3M Build System"
	@echo "================"
	@echo "Build directory: $(BUILD_DIR)"
	@echo "Available targets:"
	@echo "  all            - Build the project"
	@echo "  build          - Build the project"
	@echo "  clean          - Clean build artifacts"
	@echo "  clean-all      - Clean everything (build + test data)"
	@echo "  clean-test-data - Clean only generated test files (preserves results)"
	@echo "  rebuild        - Rebuild from scratch"
	@echo "  test           - Run comprehensive test"
	@echo "  test-main      - Run main tests"
	@echo "  install        - Install binaries"
	@echo "  info           - Show this help"
	@echo ""
	@echo "Clean Commands:"
	@echo "  make clean-test-data  - Removes generated test files, keeps results"
	@echo "  make clean-all        - Removes everything including build cache"
	@echo "  make clean            - Removes build artifacts only"

# Phony targets
.PHONY: all build clean clean-all clean-test-data rebuild test test-main install info 