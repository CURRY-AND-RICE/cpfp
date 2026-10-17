# 引数の保持・コピー・参照

cpfpでは、同じ関数でも一度にすべての引数を渡す場合と、部分適用する場合とで引数の扱いが
異なります。この文書では、cpfpが引数を保存するか、値として所有するか、参照として扱うかを
説明します。

## 最初に覚える規則

```cpp
cpfp::eq(a, b);        // その場で実行され、aとbは保存されない
auto equal_to_a = cpfp::eq(a); // aを保存し、次の呼び出しを待つ
```

引数が足りず部分適用が発生した場合、束縛済みの引数は原則として値で保存されます。左辺値でも
コピーされます。元のオブジェクトを参照させるには`std::ref`または`std::cref`を使用します。

| 渡し方 | 部分適用オブジェクトによる保持 |
| --- | --- |
| 左辺値`value` | コピーした値を所有する |
| 右辺値`T{...}` | 移動した値を所有する |
| `std::ref(value)` | 書き換え可能な参照として保持する |
| `std::cref(value)` | 読み取り専用の参照として保持する |

この規則は、cpfpが使用している`boost::hof::partial`の仕様によるものです。Boost.HOFも、既定では
値としてキャプチャし、参照としてキャプチャする場合は`std::ref`を使うと説明しています。

## 値のスナップショットと参照

左辺値を束縛すると、その時点の値が保存されます。

```cpp
std::vector<int> expected{1, 2};
auto equals_snapshot = cpfp::eq(expected);

expected.push_back(3);
bool result = equals_snapshot(expected); // false: 保存された値は{1, 2}
```

後から行われる変更を反映させる場合は`std::cref`または`std::ref`を使います。

```cpp
std::vector<int> expected{1, 2};
auto equals_current = cpfp::eq(std::cref(expected));

expected.push_back(3);
bool result = equals_current(std::vector{1, 2, 3}); // true
```

比較するだけなら`std::cref`、参照先を書き換える必要がある場合だけ`std::ref`を選ぶのが基本です。

## 一度にすべての引数を渡す場合

必要な引数を一度に渡すと、引数は部分適用オブジェクトには保存されません。ただし、コピーが
発生するかどうかは、その後に呼ばれる関数の引数型と演算の実装で決まります。「一度に渡せば
常にコピーされない」という意味ではありません。

現在のcpfpでは、次の値やrangeは参照経由で処理されます。

```cpp
cpfp::eq(xs, ys);           // xsとysを保存せず、その場で比較する
cpfp::between(lo, hi, x);   // lo、hi、xをconst参照で受け取る
cpfp::sum(xs);              // xsをforwarding referenceで受け取る
cpfp::foldl(op, init, xs);  // xsをforwarding referenceで受け取る
```

この呼び出しによってvectorそのものがコピーされることはありません。ただし、ユーザー定義の
`operator+`や`operator==`などが内部でコピーするかどうかまではcpfpは保証しません。

## API別の扱い

### 算術と比較

| API | 部分適用で保存される可能性がある引数 | 実行時の扱い |
| --- | --- | --- |
| `add`、`sub`、`mul`、`div`、`mod` | 最初のオペランド | 標準のtransparent関数オブジェクトへ転送する |
| `eq`、`ne`、`lt`、`le`、`gt`、`ge` | 最初のオペランド | 標準のtransparent関数オブジェクトへ転送する |
| `between` | 先に渡した`lo`と`hi` | 実行時は`lo`、`hi`、対象値を`const`参照で受け取る |
| `neg`、`inc`、`square`、`even`、`odd` | 通常はなし | 1引数で直ちに実行し、値を保存しない |

たとえば`between(lo, hi)(x)`では`lo`と`hi`が値として保存され、`x`は最後の呼び出しで直接
渡されます。境界値の変更を追従させるなら、次のように明示します。

```cpp
auto in_bounds = cpfp::between(std::cref(lo), std::cref(hi));
```

### 畳み込みとrange

| API | 値として扱うもの | 参照経由で扱うもの |
| --- | --- | --- |
| `sum(range)` | 値初期化したaccumulator | `range` |
| `foldl(operation, initial, range)` | `operation`と`initial` | `range` |

`foldl`の`initial`は畳み込みのaccumulatorになるため、意図的に値として扱います。部分適用した
`foldl(operation)(initial)`でも、`operation`と`initial`は生成された関数オブジェクトに保存
されます。rangeは引数順の最後にあり、実行する呼び出しで直接渡されるため、vector全体は
コピーされません。

### 関数を受け取るコンビネータ

| API | 生成される関数オブジェクトが保持するもの | 呼び出し時の値 |
| --- | --- | --- |
| `flip(function)` | `function` | 2つのオペランドは転送される。部分適用した最初のオペランドは値で保存される |
| `on(function, projection)` | `function`と`projection` | 比較対象の2値は転送される |
| `all_pass(predicates...)` | 各predicate | 対象値はコピーせず、各predicateへ同じ左辺値として渡す |
| `any_pass(predicates...)` | 各predicate | 対象値はコピーせず、各predicateへ同じ左辺値として渡す |

これらのコンビネータは、生成した関数オブジェクトを単独で安全に保持できるよう、関数やpredicateを
原則として値で所有します。状態を持つ既存の関数オブジェクトを共有したい場合は`std::ref`を
明示します。`on`で参照保持したい場合は、構成引数を一度に渡す形が明確です。

```cpp
auto compare_by = cpfp::on(std::ref(compare), std::cref(projection));
auto valid = cpfp::all_pass(std::ref(first_predicate), std::cref(second_predicate));
```

## 寿命と安全性

`std::ref`と`std::cref`はオブジェクトを所有せず、寿命も延長しません。参照先が先に破棄された
状態で生成済みの関数オブジェクトを呼ぶと、ダングリング参照になり未定義動作です。

```cpp
auto make_predicate() {
    int threshold = 10;
    return cpfp::lt(std::cref(threshold)); // 誤り: 戻った時点でthresholdが破棄される
}
```

寿命を共有できることが明らかでない場合は、既定の値保持を使用してください。また、別スレッドから
参照先を書き換える場合の同期も呼び出し側の責任です。

## 選択の目安

- 設定値のスナップショットを持たせたい: 裸の値を渡す。
- 一時オブジェクトを関数オブジェクトに所有させたい: 右辺値を渡す。
- 後からの変更を読み取らせたい: `std::cref`を使う。
- 同じ状態を持つ関数オブジェクトを書き換えながら共有したい: `std::ref`を使う。
- 寿命関係に自信がない: 値保持を選ぶ。

Boost.HOFの基礎仕様については、[partialの公式ドキュメント](https://www.boost.org/doc/libs/1_88_0/libs/hof/doc/html/include/boost/hof/partial.html)も参照してください。
