# syntax=docker/dockerfile:1

FROM ubuntu:22.04 AS build

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libevent-dev \
    libnghttp2-dev \
    libssl-dev \
    protobuf-compiler \
    libprotobuf-dev \
    libgtest-dev \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . /src

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build -j "$(nproc)"


FROM ubuntu:22.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

# Runtime dependencies for the built binary (libevent, nghttp2, openssl)
RUN apt-get update && apt-get install -y \
    libevent-2.1-7 \
    libnghttp2-14 \
    libprotobuf23 \
    libssl3 \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=build /src/build/app/app /app/app

EXPOSE 8989

ENTRYPOINT ["/app/app"]


