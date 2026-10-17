# cpfp

**C++23向けの、小さく合成可能な関数オブジェクト群です。**

このリポジトリは、v0.1.0 APIの完成版ではなく、動作するスターター実装です。
部分適用できる関数や、新しい合成の意味を持つ関数を追加します。
`std::views::transform`、`std::views::filter`、`boost::hof::flow`、
`boost::hof::compose`、`std::identity`の単なる別名は提供しません。

## 環境要件

### 必須ツール

- **GCC 15.2.0**、`-std=gnu++23`。AtCoder準拠（2026/10現在）
- **Boost 1.88.0**（コア向けのBoost.HOFと、experimental向けのBoost.Lambda2）。AtCoder準拠（2026/10現在）
- Linux、CMake 3.25以上、Ninja
- LLVM 21のclangd、clang-tidy、clang-format（コンパイラとしてではなく、エディタ・静的解析・整形にのみ使用）

AtCoderの言語環境については、[言語アップデートの一覧](https://img.atcoder.jp/file/language-update/2025-10/language-list.html)を参照してください。

### 環境変数とCMake変数

| 名前 | 種別 | 必須 | 用途・既定値 |
| --- | --- | --- | --- |
| `BOOST_ROOT` | 環境変数 | `check-env.sh`と`check.sh`では必須 | Boost 1.88.0のルートディレクトリ |
| `CLANGD` | 環境変数 | 任意 | clangdのコマンド。既定値は`clangd` |
| `CLANG_TIDY` | 環境変数 | 任意 | clang-tidyのコマンド。既定値は`clang-tidy` |
| `CLANG_FORMAT` | 環境変数 | 任意 | clang-formatのコマンド。既定値は`clang-format` |
| `CPFP_BOOST_ROOT` | CMakeキャッシュ変数 | `BOOST_ROOT`を使わず直接CMakeを実行する場合 | Boost 1.88.0のルートディレクトリ |

LLVMツールは通常、未修飾の`clangd`、`clang-tidy`、`clang-format`がLLVM 21を指すように
設定します。ディストリビューションがバージョン付きコマンドだけを提供する場合は、上記の
環境変数へ`clangd-21`などのコマンド名を指定してください。

`CPFP_BOOST_ROOT`は個別のCMake構成に対する指定です。一連の検証では、複数のCMake構成で
共有するため`BOOST_ROOT`を使用します。

競技プログラミング環境では、`CPLUS_INCLUDE_PATH`などを使って共通ライブラリを参照しても
構いません。ただし、それによってcpfp自身の依存漏れを見逃さないよう、`check.sh`は検証する
子プロセス内で`CPATH`、`CPLUS_INCLUDE_PATH`、`C_INCLUDE_PATH`、`OBJC_INCLUDE_PATH`を
解除します。呼び出し元のシェル環境は変更しません。cpfpが必要とするincludeパスはCMakeで
明示します。

`pre-commit`と`oj-bundle`は任意です。

## 使用例

```cpp
#include "cpfp.hpp"
#include <boost/hof/flow.hpp>
#include <ranges>
#include <vector>

namespace v = std::views;
namespace fp = cpfp;

std::vector xs{1, 2, 3, 4, 5, 6};
auto f = boost::hof::flow(v::filter(fp::even), v::transform(fp::square), fp::sum);
auto total = f(xs); // 56

static_assert(fp::add(2)(3) == 5);
```

`sum`と、演算・初期値を指定した`foldl`は、`|`構文でパイプラインの終端として使えます。
従来の関数呼び出しと`boost::hof::flow`による合成も利用できます。

```cpp
auto total = xs | v::filter(fp::even) | v::transform(fp::square) | fp::sum; // 56
auto product = xs | v::filter(fp::even) | fp::foldl(fp::mul)(1); // 48
auto total2 = xs | fp::foldl(fp::add, 0); // 21
```

中核となる`partial_n<N>`は、`include/cpfp/detail/partial_n.hpp`で
`boost::hof::limit`と`boost::hof::partial`の合成として実装しています。
`limit`が保証するのは引数の**最大**個数であり、一般的な意味で引数の正確な個数を
保証するものではありません。過剰な引数が誤って受理されないことは、コンパイル失敗を
期待するテストで検証しています。

### 部分適用時の引数保持

部分適用で束縛した引数は原則として値で保持されます。左辺値でもコピーされるため、元の
オブジェクトを参照させる場合は`std::ref`または`std::cref`を使用します。一度にすべての引数を
渡した場合のコピーの有無は、各APIの引数型によって決まります。

APIごとの保持方法、vectorの例、参照先の寿命については、
[`docs/ARGUMENTS.md`](docs/ARGUMENTS.md)を参照してください。

## include方針

cpfp自身のヘッダーは、`oj-bundle`で再帰的に展開できるように引用符でincludeします。

```cpp
#include "cpfp.hpp"
#include "cpfp/detail/partial_n.hpp"
```

標準ライブラリとBoostは提出環境が提供する外部ヘッダーなので、山括弧でincludeします。

```cpp
#include <boost/hof/flow.hpp>
#include <functional>
#include <ranges>
```

公開ヘッダーは、他の公開ヘッダーが先にincludeされていることを前提にせず、それぞれ単独で
コンパイルできなければなりません。この性質はヘッダー単独コンパイルテストで検証します。
clang-formatによるincludeの並べ替えは無効にしています。

## セットアップ

Boostの配置場所はリポジトリ内で仮定しません。最初に環境変数を設定し、必要なツールと
バージョンを確認します。

```sh
export BOOST_ROOT=/path/to/boost_1_88_0
bash scripts/check-env.sh
```

続いて、デバッグ構成を生成・ビルド・テストします。

```sh
cmake --fresh --preset gcc-debug
cmake --build --preset gcc-debug
ctest --preset gcc-debug
```

`BOOST_ROOT`を使用せず、このCMake構成にだけBoostの場所を指定することもできます。

```sh
cmake --fresh --preset gcc-debug -DCPFP_BOOST_ROOT=/path/to/boost_1_88_0
```

`build/debug/compile_commands.json`も生成され、clangdがコンパイル条件を参照できるように
なります。生成後にエディタの診断が更新されない場合は、clangdを再起動してください。

対象環境との互換性を保証せず、別バージョンのGCCやBoostでローカル確認だけを行う場合は、
厳密なバージョン検査を無効にした`gcc-local`プリセットを使用できます。

```sh
cmake --preset gcc-local
cmake --build --preset gcc-local
ctest --preset gcc-local
```

## 検証とコード整形

すべてのビルド構成、正常系・異常系コンパイルテスト、clang-format、clang-tidyを
まとめて検証するには、次を実行します。

```sh
bash scripts/check.sh
```

ソースコード全体へclang-formatを適用するには、次を実行します。

```sh
bash scripts/format.sh
```

整形対象は`include/`、`tests/`、`examples/`内の`.hpp`と`.cpp`です。

GitHub Actionsではpush、pull request、手動実行のたびに、GCC 15.2.0、Boost 1.88.0、
LLVM 21を使って同じ`check.sh`を実行します。GCCとBoostはダウンロード内容のSHA-256を
検証し、次回以降の実行に向けてキャッシュします。Dockerは使用しません。

pre-commitを使用する場合、通常のコミット時にはclang-format 21の検査だけを実行します。
時間のかかる全検証は手動ステージです。

```sh
pre-commit install
pre-commit run --all-files
pre-commit run --hook-stage manual full-project-check
```

`oj-bundle`を使用する場合は、導入済みの環境を有効にしてから実行してください。

## AtCoderへの提出

AtCoderにはBoostがありますが、cpfpはインストールされていません。そのため、
`#include "cpfp.hpp"`を含むコードはそのまま提出できません。cpfpのヘッダーだけを
ソースへ展開し、標準ライブラリとBoostのincludeを残した単一の翻訳単位を生成します。

`oj-bundle`を使用する場合は、次を実行します。

```sh
oj-bundle -I include examples/ex1.cpp > build/submission.cpp
```

cpfpのヘッダーは`#include "..."`、標準ライブラリとBoostは`#include <...>`で記述して
いるため、`oj-bundle`はcpfpだけを展開します。

生成された`build/submission.cpp`を提出画面へ貼り付けてください。

## 対象範囲

現在の主要な関数は、`add`、`sub`、`mul`、`div`、`mod`、`neg`、`inc`、`square`、
`eq`、`ne`、`lt`、`le`、`gt`、`ge`、`even`、`odd`、`between`、`flip`、`on`、
`all_pass`、`any_pass`、`foldl`、`sum`です。今後は、部分適用または実質的な機能を
追加する関数だけを採用します。独立した`cpfp::experimental`モジュールは、optional、
expected、applicative、generator向けに予約しています。

引数の保持・コピー・参照については`docs/ARGUMENTS.md`、制約、ライフサイクル、採用済みの
設計判断については`docs/DESIGN.md`を参照してください。

## ライセンス

BSL-1.0です。詳細は`LICENSE_1_0.txt`を参照してください。
