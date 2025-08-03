#!/bin/bash

# R3M Test Data Cleaner
# =====================
# This script cleans generated test files while preserving results and reports

DATA_DIR="./data"

echo "🧹 Cleaning generated test files..."

# Remove all generated test files but preserve results and reports
find "$DATA_DIR" -maxdepth 1 -type f \( \
    -name "*.txt" -o \
    -name "*.md" -o \
    -name "*.json" -o \
    -name "*.csv" -o \
    -name "*.log" -o \
    -name "*.html" -o \
    -name "*.pdf" -o \
    -name "*.xml" -o \
    -name "*.yml" -o \
    -name "*.yaml" -o \
    -name "*.tsv" -o \
    -name "*.conf" -o \
    -name "*.mdx" \
\) -not -name "test_results.txt" -delete

# Clean chunks subdirectory
if [ -d "$DATA_DIR/chunks" ]; then
    echo "🧹 Cleaning chunks directory..."
    rm -rf "$DATA_DIR/chunks"
    echo "✅ Chunks directory cleaned!"
fi

echo "✅ Test data cleaned! Results and reports preserved."
echo "📁 Remaining files in data/:"
ls -la "$DATA_DIR" 2>/dev/null || echo "   (data directory is empty)" 