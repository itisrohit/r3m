---
title: R3M Document Processing API
emoji: 🚀
colorFrom: blue
colorTo: purple
sdk: docker
pinned: false
---

# R3M Document Processing API

High-performance C++ document processing and retrieval system deployed on Hugging Face Spaces.

## Features

- SIMD-optimized text processing
- Parallel document processing with thread pools
- Memory-efficient chunking algorithms
- Multi-format document support (PDF, DOCX, TXT)
- RESTful API endpoints

## API Endpoints

- `GET /` - API information
- `GET /health` - Health check
- `GET /info` - Detailed API information
- `POST /process` - Process single document
- `POST /batch` - Process batch of documents

## Hardware Requirements

- **CPU**: 2 vCPU (free tier)
- **Memory**: 16GB RAM
- **Storage**: 50GB disk space

## Environment Variables

- `DOCKER_CONTAINER=true` - Docker environment flag
- `R3M_ENABLE_MEMORY_POOLING=false` - Disable memory pooling for stability
- `R3M_MAX_WORKERS=4` - Maximum worker threads
- `R3M_OPTIMAL_BATCH_SIZE=100` - Optimal batch size for processing

## Deployment

This Space automatically builds and deploys the R3M C++ Docker container on Hugging Face's infrastructure. 