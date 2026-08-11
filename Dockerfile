from debian:trixie as base

run apt update
run apt install -y \
  build-essential \
  pkg-config \
  git \
  wayland-protocols \
  libxkbcommon-dev \
  libwayland-dev \
  libcairo2-dev

entrypoint bash

from base as tooling

run apt update
run apt install -y \
  clangd
workdir /src
entrypoint bash
