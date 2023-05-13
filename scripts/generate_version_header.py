#!/usr/bin/env python3

import os
import subprocess
import sys
from typing import Tuple
import argparse

"""Gitのコミットハッシュとコミット日時をCソースファイルに書き込む

コミットハッシュやコミット日時を書き込む場所は、以下のようにする
```
// 書き込み前
#define COMMIT_HASH "!!!COMMIT_HASH!!!"
#define COMMIT_DATE "!!!COMMIT_DATE!!!"
// 書き込み後
#define COMMIT_HASH "09fff368d273878896277311ac4a01604eedc3ca"
#define COMMIT_DATE "2023-05-05T21:57:16+09:00"
```
"""


def main():
    repo_path, base, output = parse_args()

    if is_git_repository_root(repo_path) == False:
        print("Not Git repository root", file=sys.stderr)
        generate_header(base,output, "Not Git repository", "Not Git repository")
        sys.exit(1)

    hash, date = commit_info(repo_path)

    generate_header(base, output, hash, date)


def parse_args() -> Tuple[str, str, str]:
    """
    コマンドライン引数をパースする

    引数のパスが存在しない場合は、エラーを出力して終了する

    Returns:
        Tuple[str, str]: パスと元になるファイルと、出力先ファイルのパス
    """

    parser = argparse.ArgumentParser(description="与えらえたファイルにGitのコミットハッシュとコミット日時を書き込む")

    parser.add_argument(
        "repo_path",
        type=str,
        help="Gitリポジトリのパス",
    )

    parser.add_argument(
        "base",
        type=str,
        help="ベースファイルのパス",
    )

    parser.add_argument(
        "output",
        type=str,
        help="出力ファイルのパス",
    )

    args = parser.parse_args()

    def exists(path: str):
        if os.path.exists(path) == False:
            print("Not file: " + path, file=sys.stderr)
            sys.exit(1)

    exists(args.repo_path)
    exists(args.base)
    exists(os.path.dirname(args.output))

    return args.repo_path, args.base, args.output


def is_git_repository_root(path: str) -> bool:
    """Gitリポジトリのルートディレクトリかどうかを判定する

    Args:
        path (str): パス

    Returns:
        bool: Gitリポジトリのルートディレクトリかどうか
    """

    try:
        git_root = subprocess.check_output(
            ["git", "rev-parse", "--show-toplevel"], encoding="utf-8"
        ).strip()
    except subprocess.CalledProcessError:
        return False

    if git_root != path:
        return False

    return True


def commit_info(path: str) -> Tuple[str, str]:
    """Gitリポジトリの情報を返す

    現在は、HEADのコミットハッシュ(hash)とコミット日時(date)を返している

    Args:
        path (str): パス

    Returns:
        Tuple[str, str]: コミットハッシュとコミット日時
    """

    hash = subprocess.check_output(
        ["git", "log", "-1", "--pretty=format:%H"],
        cwd=path,
    ).decode("utf-8")

    date = subprocess.check_output(
        ["git", "log", "-1", "--pretty=format:%cI"],
        cwd=path,
    ).decode("utf-8")

    return hash, date


def generate_header(base:str, path: str, hash: str, date: str):
    """与えられたCソースファイルにコミットハッシュとコミット日時を書き込む

    コミットハッシュやコミット日時を書き込む場所は、以下のようにする
    ```
    // 書き込み前
    #define COMMIT_HASH "!!!COMMIT_HASH!!!"
    #define COMMIT_DATE "!!!COMMIT_DATE!!!"
    // 書き込み後
    #define COMMIT_HASH "09fff368d273878896277311ac4a01604eedc3ca"
    #define COMMIT_DATE "2023-05-05T21:57:16+09:00"
    ```

    Args:
        path (str): パス
        hash (str): コミットハッシュ
        date (str): コミット日時
    """

    write_lines: list[str] = []
    with open(base, mode="r") as f:
        lines = f.readlines()
        for line in lines:
            line = line.replace("!!!COMMIT_HASH!!!", hash)
            line = line.replace("!!!COMMIT_DATE!!!", date)
            write_lines.append(line)

    # 書き込みたいファイルの中身が変わらない場合は、書き込みをスキップする
    with open(path, mode="r") as f:
        if f.readlines() == write_lines:
            return

    with open(path, mode="w") as f:
        f.writelines(write_lines)


if __name__ == "__main__":
    main()
