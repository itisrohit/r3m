# R3M Production Dockerfile
# Multi-stage build for optimal performance and consistency

# Stage 1: Build environment
FROM ubuntu:22.04 AS builder

# Set environment variables for consistent builds
ENV DEBIAN_FRONTEND=noninteractive
ENV CMAKE_BUILD_TYPE=Release
ENV CXXFLAGS="-O3 -march=native -DNDEBUG"
ENV CFLAGS="-O3 -march=native -DNDEBUG"

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
    make -j$(nproc) && \
    make install && \
    ldconfig && \
    rm -rf /tmp/crow

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Create build directory and build (clean approach)
RUN rm -rf build && mkdir -p build && cd build \
    && cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DR3M_ENABLE_SIMD=ON \
        -DR3M_ENABLE_PARALLEL=ON \
        -DR3M_ENABLE_HTTP_SERVER=ON \
    && make -j$(nproc)

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
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN groupadd -r r3m && useradd -r -g r3m r3m

# Set working directory
WORKDIR /app

# Copy executables from builder stage
COPY --from=builder /app/build/r3m /app/r3m
COPY --from=builder /app/build/r3m-test /app/r3m-test
COPY --from=builder /app/build/r3m-simd-test /app/r3m-simd-test
COPY --from=builder /app/build/r3m-parallel-optimization-test /app/r3m-parallel-optimization-test
COPY --from=builder /app/build/r3m-performance-benchmark /app/r3m-performance-benchmark

# Copy Crow libraries and headers
COPY --from=builder /usr/local/lib/libcrow* /usr/local/lib/
COPY --from=builder /usr/local/include/crow /usr/local/include/crow
RUN ldconfig

# Copy configuration and data
COPY --from=builder /app/configs/ /app/configs/
# COPY --from=builder /app/data/ /app/data/  # Removed - data directory doesn't exist in builder

# Set ownership
RUN chown -R r3m:r3m /app

# Set environment variable to disable memory pooling in Docker
ENV DOCKER_CONTAINER=true

# Switch to non-root user
USER r3m

# Expose port
EXPOSE 8080

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# Default command
CMD ["./r3m", "--port", "8080", "--host", "0.0.0.0"]

# Stage 3: Development environment (optional)
FROM runtime AS dev

# Install development tools
USER root
RUN apt-get update && apt-get install -y \
    gdb \
    valgrind \
    strace \
    && rm -rf /var/lib/apt/lists/*

# Switch back to non-root user
USER r3m

# Development command
CMD ["./r3m", "--port", "8080", "--host", "0.0.0.0", "--debug"] 