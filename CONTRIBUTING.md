# Contribution Guide

## Issue

不具合や改善案について、報告してください

基本的になんでもかまいません

気軽に聞きたいときは [Discussions](https://github.com/yutotnh/spirit/discussions) に投稿してもかまいません

## PullRequest

反映してほしい修正を送ってください

規模も小さく、開発立ち上げ直後なので、関数名の名前変更や構造自体の変更などの**破壊的な変更**の提案も積極的に受け付けています

Issue の作成や議論後に Pull request を出しても良いですが、いきなり Pull request を出しても構いません

## Code style

基本的には、既存のコードの書き方を踏襲して書いてください

いくつかのお願いを以下に記します

他にもお願いをする場合がありますが、ご了承のほどよろしくお願いいたします

### 命名規則

- PascalCase
  - class
  - enum, enum class
- snake_case
  - namespace
  - function
  - const

### その他

- ヘッダファイルの拡張子は `.h` 、ソースファイルの拡張子は `.cpp`
- `if` , `for` , `while` 中の処理が1行でも `{ ... }` を利用する
- private なメンバ変数は先頭に `_` をつける
- public な関数には Doxygen 形式のコメントを付ける
- ClangFormat を使って自動整形する
- cspell を使ってスペルチェックする
  - 辞書に登録されていない単語は、辞書に登録する
