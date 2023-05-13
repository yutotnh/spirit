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
    repo_path, output = parse_args()

    if is_git_repository_root(repo_path) == False:
        print("Not Git repository root", file=sys.stderr)
        modify_file(output, "Not Git repository", "Not Git repository")
        sys.exit(1)

    hash, date = commit_info(repo_path)

    modify_file(output, hash, date)


def parse_args() -> Tuple[str, str]:
    """
    コマンドライン引数をパースする

    引数のパスが存在しない場合は、エラーを出力して終了する

    Returns:
        Tuple[str, str]: パスと出力ファイルのパス
    """

    parser = argparse.ArgumentParser(description="与えらえたファイルにGitのコミットハッシュとコミット日時を書き込む")

    parser.add_argument(
        "repo_path",
        type=str,
        help="Gitリポジトリのパス",
    )

    parser.add_argument(
        "output",
        type=str,
        help="出力ファイルのパス",
    )

    args = parser.parse_args()

    if os.path.exists(args.repo_path) == False:
        print("Not found path: " + args.repo_path, file=sys.stderr)
        sys.exit(1)

    if os.path.exists(args.output) == False:
        print("Not found path: " + args.output, file=sys.stderr)
        sys.exit(1)

    return args.repo_path, args.output


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


def modify_file(path: str, hash: str, date: str):
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
    with open(path, mode="r") as f:
        lines = f.readlines()
        for line in lines:
            line = line.replace("!!!COMMIT_HASH!!!", hash)
            line = line.replace("!!!COMMIT_DATE!!!", date)
            write_lines.append(line)

    with open(path, mode="w") as f:
        f.writelines(write_lines)


if __name__ == "__main__":
    main()
