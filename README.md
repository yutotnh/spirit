<div align="center">

<img src="./images/icon.svg" width="200">

[![Build spirit](https://github.com/yutotnh/spirit/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/yutotnh/spirit/actions/workflows/build.yml)
[![Testing With CMake and CTest](https://github.com/yutotnh/spirit/actions/workflows/cmake-test.yml/badge.svg?branch=main)](https://github.com/yutotnh/spirit/actions/workflows/cmake-test.yml)
[![CodeQL](https://github.com/yutotnh/spirit/actions/workflows/codeql.yml/badge.svg)](https://github.com/yutotnh/spirit/actions/workflows/codeql.yml)

[![Dependency Review](https://github.com/yutotnh/spirit/actions/workflows/dependency-review.yml/badge.svg)](https://github.com/yutotnh/spirit/actions/workflows/dependency-review.yml)
[![Deploy static content to Pages](https://github.com/yutotnh/spirit/actions/workflows/deploy-static.yml/badge.svg)](https://github.com/yutotnh/spirit/actions/workflows/deploy-static.yml)

</div>

# Spirit

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

最終的にはMbed/Arduino等での利用を想定していますが、自動テストを非マイコンで行う都合上、可能な限り特定の環境への依存を減らしています

## Development

### Test

Google Testを使っています

### Build & Test

下の手順でビルドからテストの実行までできます

```shell
cd $(git rev-parse --show-superproject-working-tree --show-toplevel | head -1) # リポジトリのルートディレクトリに移動
cmake -S . -B build # Configurate & Generate
cmake --build build # Build
ctest --test-dir build # Test
```

### Development Containers

このリポジトリは、Visual Studio Code Dev Containers/GitHub Codespaces を含んでいます

Development Containers を利用することで、本プロジェクトでの開発に必要な最低限の環境を自動的に構築し、直ちに開発に取り組むことができます。

使い方等の詳細は、 [Developing inside a Container](https://code.visualstudio.com/docs/devcontainers/containers) をご覧ください

## Document

Doxygenを使って、ソースコードからドキュメントを生成しています

生成したドキュメントへのリンクは以下です。

- [一般ユーザ用](https://yutotnh.github.io/spirit/general)
- [spirit開発者用(全部入り)](https://yutotnh.github.io/spirit)

## Contributing

PullRequestやissueを送る際は、 [CONTRIBUTING.md](./CONTRIBUTING.md) をご覧ください
