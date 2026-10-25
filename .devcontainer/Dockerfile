FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Development tools and C++ dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    git \
    pkg-config \
    ca-certificates \
    curl \
    libasio-dev \
    libssl-dev \
    zlib1g-dev \
    libboost-all-dev \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Build and install Wt from source
RUN git clone --depth 1 --branch 4.14.4 \
    https://github.com/emweb/wt.git /tmp/wt \
    && cmake -S /tmp/wt -B /tmp/wt/build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
        -DBUILD_EXAMPLES=OFF \
        -DBUILD_TESTS=OFF \
    && cmake --build /tmp/wt/build --parallel 2 \
    && cmake --install /tmp/wt/build \
    && ldconfig \
    && rm -rf /tmp/wt

WORKDIR /workspace