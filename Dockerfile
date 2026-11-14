# Multi-stage Docker build for C++17 GENOME·AUTOMATA Engine
FROM gcc:14-bookworm AS builder

# Install CMake
RUN apt-get update && apt-get install -y --no-install-recommends cmake && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Build C++ backend using CMake with optimizations
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --target dna_server -j$(nproc)

# Final minimal runtime image
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates libstdc++6 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy binary and frontend assets from builder
COPY --from=builder /app/build/backend/dna_server /app/dna_server
COPY --from=builder /app/frontend /app/frontend

# Default port
ENV PORT=8080
EXPOSE 8080

CMD ["/app/dna_server"]
