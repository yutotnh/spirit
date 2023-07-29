FROM mcr.microsoft.com/devcontainers/cpp:0-ubuntu-22.04

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="none"

# Optionally install the cmake for vcpkg
COPY ./reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

# [Optional] Uncomment this section to install additional vcpkg ports.
# RUN su vscode -c "${VCPKG_ROOT}/vcpkg install <your-port-name-here>"

# [Optional] Uncomment this section to install additional packages.
# [Optional] Uncomment this section to install additional packages.
# 下記のソフトウェアを使うために必要なパッケージをインストールする
# - doxygen
# - clang-format
# - cspell
RUN curl -sL https://deb.nodesource.com/setup_lts.x | bash - \
    && apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends graphviz \
                                                  doxygen \
                                                  clang-format \
                                                  nodejs \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* \
    && npm -g install cspell

USER vscode

COPY ./script/* /tmp/

RUN /tmp/install-arduino-cli.sh && \
    # PATHを設定しているため現行環境でシェルを実行する
    . /tmp/install-gcc-arm-none-eabi.sh && \
    /tmp/install-mbed-cli-1.sh && \
    /tmp/install-mbed-cli-2.sh
