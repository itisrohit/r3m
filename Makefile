# R3M Makefile - Simplified build and test commands
# ================================================

.PHONY: build clean rebuild test clean-all clean-test-data info help

# Default target
all: build

# Build the project
build:
	@echo "🔨 Building R3M..."
	mkdir -p build
	cd build && cmake ..
	cd build && make
	@echo "✅ Build completed!"

# Clean build artifacts
clean:
	@echo "🧹 Cleaning build artifacts..."
	rm -rf build/
	@echo "✅ Clean completed!"

# Clean all (including test data)
clean-all:
	@echo "🧹 Cleaning all artifacts and test data..."
	rm -rf build/
	./scripts/clean_test_data.sh
	@echo "✅ Clean all completed!"

# Clean only test data (preserve results)
clean-test-data:
	@echo "🧹 Cleaning test data files..."
	./scripts/clean_test_data.sh
	@echo "✅ Test data cleaned!"

# Rebuild everything
rebuild: clean build

# Run tests
test:
	@echo "🧪 Running comprehensive tests..."
	mkdir -p build
	cd build && cmake ..
	cd build && make test
	@echo "✅ Tests completed!"

# Show project info
info:
	@echo "📋 R3M Project Information"
	@echo "=========================="
	@echo "Available commands:"
	@echo "  make build          - Build the project"
	@echo "  make clean          - Clean build artifacts"
	@echo "  make clean-all      - Clean everything including test data"
	@echo "  make clean-test-data - Clean only test data (preserve results)"
	@echo "  make rebuild        - Clean and rebuild"
	@echo "  make test           - Run comprehensive tests"
	@echo "  make info           - Show this information"
	@echo ""
	@echo "Configuration files:"
	@echo "  configs/dev/config.yaml   - Development configuration"
	@echo "  configs/prod/config.yaml  - Production configuration"
	@echo ""
	@echo "Test results:"
	@echo "  data/test_results.txt     - Latest test results"

# Help target
help: info 