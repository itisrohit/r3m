#!/bin/bash

# Upload only essential files to avoid rate limits
echo "Uploading essential files to Hugging Face Space..."

# Essential files for Docker deployment
ESSENTIAL_FILES=(
    "Dockerfile"
    "README.md"
    "CMakeLists.txt"
    "src/"
    "include/"
    "configs/"
    "examples/"
    "tests/"
    "scripts/"
    "data/"
)

# Upload each essential component separately
for file in "${ESSENTIAL_FILES[@]}"; do
    if [ -e "$file" ]; then
        echo "Uploading $file..."
        hf upload anorohit/r3m-api "$file" --token "$HF_TOKEN"
        sleep 5  # Small delay between uploads
    fi
done

echo "Essential files uploaded successfully!" 