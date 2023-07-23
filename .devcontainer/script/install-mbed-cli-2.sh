#!/usr/bin/env bash
#
# Mbed CLI 2 のインストール
#
# 参考: https://os.mbed.com/docs/mbed-os/v6.16/build-tools/install-or-upgrade.html
#

set -eu

# Mbed CLI 2 に必要なパッケージをインストール
sudo apt-get update
sudo apt-get -y install --no-install-recommends python3 \
    python3-pip \
    gcc-arm-none-eabi

sudo apt-get clean
sudo rm -rf /var/lib/apt/lists/*

# Installing Mbed CLI 2
python -m pip install mbed-tools
