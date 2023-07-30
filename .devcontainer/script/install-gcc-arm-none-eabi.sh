#!/usr/bin/env bash
#
# GNU Arm Embedded Toolchain のインストール
#
# 参考: https://developer.arm.com/downloads/-/gnu-rm

set -eu

# GNU Arm Embedded Toolchain のバージョン
GCC_ARM_VERSION='10.3-2021.07'

machine=$(uname -m)
if [ "${machine}" = "x86_64" ] || [ "${machine}" = "aarch64" ]; then
    url="https://developer.arm.com/-/media/Files/downloads/gnu-rm/${GCC_ARM_VERSION}/gcc-arm-none-eabi-${GCC_ARM_VERSION}-${machine}-linux.tar.bz2"
else
    echo "Unsupported architecture: ${machine}"
    exit 1
fi

# Check if the file already exists
if [ -f "/usr/local/gcc-arm-none-eabi-${GCC_ARM_VERSION}/bin/arm-none-eabi-gcc" ]; then
    echo "gcc-arm-none-eabi-${GCC_ARM_VERSION} is already installed."
    exit 0
fi

wget "${url}" -P /tmp/
tar -xvf "/tmp/gcc-arm-none-eabi-${GCC_ARM_VERSION}-*-linux.tar.bz2" -C /tmp/
sudo mv "/tmp/gcc-arm-none-eabi-${GCC_ARM_VERSION}" /usr/local/
rm "/tmp/gcc-arm-none-eabi-${GCC_ARM_VERSION}-*-linux.tar.bz2"

sudo ln -s "/usr/local/gcc-arm-none-eabi-${GCC_ARM_VERSION}/bin/*" /usr/local/bin/
