# Multi-stage build for R3M API on Hugging Face Spaces
FROM ubuntu:22.04 AS builder

# Set environment variables for build optimization
ENV DEBIAN_FRONTEND=noninteractive
ENV MAKEFLAGS="-j2"
ENV MALLOC_ARENA_MAX=2

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libboost-all-dev \
    libpoppler-cpp-dev \
    libgumbo-dev \
    libasio-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build the application
RUN mkdir build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j2 \
    && strip r3m

# Runtime stage
FROM ubuntu:22.04 AS runtime

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libpoppler-cpp0v5 \
    libgumbo1 \
    libboost-system1.74.0 \
    libboost-thread1.74.0 \
    libboost-filesystem1.74.0 \
    libasio3 \
    libnlohmann-json3v5 \
    ca-certificates \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Create app user
RUN useradd -m -u 1000 app

# Set working directory
WORKDIR /app

# Copy built application
COPY --from=builder /app/build/r3m .

# Set environment variables for Docker container
ENV DOCKER_CONTAINER=true
ENV R3M_ENABLE_MEMORY_POOLING=false
ENV R3M_MAX_WORKERS=4
ENV R3M_OPTIMAL_BATCH_SIZE=100

# Change ownership
RUN chown -R app:app /app

# Switch to app user
USER app

# Expose port (Hugging Face Spaces expects 7860)
EXPOSE 7860

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:7860/health || exit 1

# Start the application
CMD ["./r3m"] 