<div align="center">

![Icon](./images/icon.svg)

[![Build spirit](https://github.com/yutotnh/spirit/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/yutotnh/spirit/actions/workflows/build.yml)
[![Testing With CMake and CTest](https://github.com/yutotnh/spirit/actions/workflows/cmake-test.yml/badge.svg?branch=main)](https://github.com/yutotnh/spirit/actions/workflows/cmake-test.yml)
[![CodeQL](https://github.com/yutotnh/spirit/actions/workflows/codeql.yml/badge.svg?=branch=main)](https://github.com/yutotnh/spirit/actions/workflows/codeql.yml)

[![Dependency Review](https://github.com/yutotnh/spirit/actions/workflows/dependency-review.yml/badge.svg?=branch=main)](https://github.com/yutotnh/spirit/actions/workflows/dependency-review.yml)
[![Deploy static content to Pages](https://github.com/yutotnh/spirit/actions/workflows/deploy-static.yml/badge.svg?=branch=main)](https://github.com/yutotnh/spirit/actions/workflows/deploy-static.yml)
[![CodeFactor](https://www.codefactor.io/repository/github/yutotnh/spirit/badge)](https://www.codefactor.io/repository/github/yutotnh/spirit)

</div>

# 👻 spirit

モータードライバーを駆動するためのライブラリ

下の図のように、マイコン間の通信を行いモータードライバーを制御することを目的としています

```mermaid
flowchart TB
    A[Controller]
    A -- CAN --> B["Peripheral\n(Motor Driver)"] --> B2[Motor]
    A -- CAN --> C["Peripheral\n(Motor Driver)"] --> C2[Motor]
    A -- CAN --> D["Peripheral\n(Motor Driver)"] --> D2[Motor]
    A -- CAN --> E["Peripheral\n(Motor Driver)"] --> E2[Motor]
```

最終的にはMbed CE(Mbed OSのコミュニティ版)/Arduino等での利用を想定していますが、自動テストを非マイコンで行う都合上、可能な限り特定の環境への依存を減らしています

## 💻 Supported platforms

下記の環境では多分動きます

| Platform  |   architecture    |
| :-------: | :---------------: |
|  Arduino  |       ESP32       |
|  Arduino  |     Portenta      |
|  Arduino  | Raspberry Pi Pico |
|  Arduino  |       STM32       |
|  Mbed CE  |      LPCxxxx      |
|  Mbed CE  |       STM32       |

### 🔧 Mbed CE

Mbed OS は Arm の保守が2026年7月に終了したため、[Mbed CE](https://github.com/mbed-ce/mbed-os)(Mbed OSのコミュニティ版)を使います

Mbed CE のプロジェクトで使うときは、`mbed-os` を `add_subdirectory()` した後に、このリポジトリを `add_subdirectory()` し、実行ファイルに `spirit` と `spirit-platform-mbed` をリンクしてください

プロジェクトの例は [.github/workflows/build-mbed](./.github/workflows/build-mbed) にあります

## ⌨️ Development

### 🛠️ Build & Test

リポジトリのルートディレクトリで下の手順を行うことで、ビルドからテストの実行ができます

プリセットは `Debug` と `Release` の2種類があります

```shell
cmake -S . --preset Debug    # Configurate & Generate
cmake --build --preset Debug # Build
ctest --preset Debug         # Test
```

テストフレームワークとして Google Test を利用しています

### 🐋 Development Containers

このリポジトリは、Visual Studio Code Dev Containers/GitHub Codespaces を含んでいます

Development Containers を利用することで、本プロジェクトでの開発に必要な最低限の環境を自動的に構築し、直ちに開発に取り組むことができます

使い方等の詳細は、 [Developing inside a Container](https://code.visualstudio.com/docs/devcontainers/containers) をご覧ください

## 📖 Document

Doxygenを使って、ソースコードからドキュメントを生成しています

生成したドキュメントへのリンクは以下です

- [一般ユーザ用](https://yutotnh.github.io/spirit/general)
  - Controller側で利用するAPIのみをまとめたページ
- [開発者用](https://yutotnh.github.io/spirit)
  - Controller/Peripheral側の全てのAPIをまとめたページ(=全部入り)

リポジトリのルートディレクトリで下の手順を行うことで、各ドキュメントの生成ができます

```shell
git submodule update --init                              # For doxygen-awesome-css
doxygen ${workspaceFolder}/doxygen/Doxyfile              # Generate developer documentation for spirit
doxygen ${workspaceFolder}/doxygen/Doxyfile4GeneralUsers # Generate general user documentation
```

## 🤝 Contributing

PullRequestやissueを送る際は、 [CONTRIBUTING.md](./CONTRIBUTING.md) をご覧ください

## 🏃 Using spirit

- 🚧 [yutotnh/spirit-mbed-can-motor-controller](https://github.com/yutotnh/spirit-mbed-can-motor-controller)
  - spirit と CAN を用いた Mbed の モータードライバ制御プログラム(Controller側)
- [yutotnh/spirit-mbed-can-motor-driver](https://github.com/yutotnh/spirit-mbed-can-motor-driver.git)
  - spirit と CAN を用いた Mbed の モータードライバ制御プログラム(Peripheral側)
