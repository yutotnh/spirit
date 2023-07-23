#!/usr/bin/env bash
#
# Mbed CLI 1 のインストール
#
# 参考: https://os.mbed.com/docs/mbed-os/v6.16/build-tools/install-and-set-up.html

set -eu

# Mbed CLI 1 に必要なパッケージをインストール
sudo apt-get update
sudo apt-get -y install --no-install-recommends python3 \
    python3-pip \
    python3-dev \
    git \
    mercurial \
    gcc-arm-none-eabi

sudo apt-get clean
sudo rm -rf /var/lib/apt/lists/*

# Installing Mbed CLI 1
python3 -m pip install mbed-cli
