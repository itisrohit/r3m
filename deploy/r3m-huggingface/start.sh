#!/bin/bash

# Startup script for Hugging Face Spaces
echo "Starting R3M API on Hugging Face Spaces..."

# Set environment variables
export DOCKER_CONTAINER=true
export R3M_PORT=7860
export R3M_HOST=0.0.0.0

# Debug: Check if config file exists
echo "Checking config file..."
ls -la /app/configs/dev/
cat /app/configs/dev/config.yaml

# Start the application
echo "Starting server..."
exec ./r3m 