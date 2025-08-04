#!/bin/bash

# Startup script for Hugging Face Spaces
echo "Starting R3M API on Hugging Face Spaces..."

# Set environment variables
export DOCKER_CONTAINER=true
export R3M_PORT=7860
export R3M_HOST=0.0.0.0

# Start the application on port 7860
echo "Starting server on port 7860..."
exec ./r3m --port 7860 --host 0.0.0.0 