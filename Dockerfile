FROM debian:stable-slim

RUN apt-get update && apt-get install -y \
    g++ \
    -std=gnu++23 \
    libcurl4-openssl-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY src/ ./src/

RUN g++ src/*.cpp -o app -lcurl

CMD ["./app"]