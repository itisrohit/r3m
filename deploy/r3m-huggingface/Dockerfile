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
    pkg-config \
    libssl-dev \
    libcurl4-openssl-dev \
    libboost-all-dev \
    libpoppler-cpp-dev \
    libgumbo-dev \
    nlohmann-json3-dev \
    libasio-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Crow from source
RUN git clone https://github.com/CrowCpp/Crow.git /tmp/crow && \
    cd /tmp/crow && \
    mkdir build && cd build && \
    cmake .. -DCROW_BUILD_EXAMPLES=OFF -DCROW_BUILD_TESTS=OFF && \
    make -j2 && \
    make install && \
    ldconfig && \
    rm -rf /tmp/crow

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build the application
RUN rm -rf build && mkdir -p build && cd build \
    && cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DR3M_ENABLE_SIMD=ON \
        -DR3M_ENABLE_PARALLEL=ON \
        -DR3M_ENABLE_HTTP_SERVER=ON \
    && make -j2

# Runtime stage
FROM ubuntu:22.04 AS runtime

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libssl3 \
    libcurl4 \
    libboost-system1.74.0 \
    libboost-thread1.74.0 \
    libboost-filesystem1.74.0 \
    libpoppler-cpp0v5 \
    libgumbo1 \
    ca-certificates \
    curl \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Create app user
RUN useradd -m -u 1000 app

# Set working directory
WORKDIR /app

# Copy executables from builder stage
COPY --from=builder /app/build/r3m /app/r3m

# Copy Crow libraries and headers
COPY --from=builder /usr/local/lib/libcrow* /usr/local/lib/
COPY --from=builder /usr/local/include/crow /usr/local/include/crow
RUN ldconfig

# Copy configuration and data
COPY --from=builder /app/configs/ /app/configs/
COPY deploy/r3m-huggingface/config.yaml /app/configs/dev/config.yaml
COPY deploy/r3m-huggingface/start.sh /app/start.sh

# Debug: List the config files to verify they exist
RUN ls -la /app/configs/dev/ && cat /app/configs/dev/config.yaml

# Set environment variables for Docker container
ENV DOCKER_CONTAINER=true
ENV R3M_PORT=7860
ENV R3M_HOST=0.0.0.0
ENV PORT=7860

# Change ownership
RUN chown -R app:app /app

# Switch to app user
USER app

# Expose port (Hugging Face Spaces expects 7860)
EXPOSE 7860

# Health check (Hugging Face Spaces expects port 7860)
HEALTHCHECK --interval=30s --timeout=10s --start-period=10s --retries=5 \
    CMD curl -f http://localhost:7860/health || exit 1

# Default command (use startup script for debugging)
CMD ["./start.sh"] 