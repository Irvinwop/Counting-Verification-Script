FROM debian:stable-slim

RUN apt-get update && apt-get install -y \
    g++ \
    curl \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /uwu

COPY src/ ./src/

RUN g++ -std=c++23 src/main.cpp -o uwu

CMD ["./uwu"]