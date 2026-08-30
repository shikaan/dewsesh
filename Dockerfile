from debian:trixie as base

run apt update

# build tools
run apt install -y \
  build-essential \
  pkg-config \
  git \
  scdoc 

# dependencies
run apt install -y \
  libwayland-dev \
  libcairo2-dev \
  libfreetype-dev

entrypoint bash

from base as tooling

# development tools
run apt install -y clangd

workdir /src
entrypoint bash
