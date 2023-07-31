#!/usr/bin/env bash
#
# Arduino CLI のインストールとセットアップをする
#
# 参考: https://arduino.github.io/arduino-cli/0.33/installation/

set -eu

# Arduino CLI に必要なパッケージをインストール
sudo apt-get update
sudo apt-get -y install --no-install-recommends python3 \
    python3-dev \
    python-is-python3 \
    python3-pip

sudo apt-get clean
sudo rm -rf /var/lib/apt/lists/*

pip3 install pyserial \
    esptool

# Installing Arduino CLI
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sudo BINDIR=/usr/local/bin sh

# Setup bash completion
echo 'source <(arduino-cli completion bash)' >>~/.bashrc

# Install supported boards
arduino-cli config init

# ESP32
arduino-cli config add board_manager.additional_urls https://dl.espressif.com/dl/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32

# STM32
arduino-cli config add board_manager.additional_urls https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
arduino-cli core update-index
arduino-cli core install STMicroelectronics:stm32

# Mbed
arduino-cli core update-index
arduino-cli core install arduino:mbed
