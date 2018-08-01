# 可変窓を用いた高速再帰的スペクトル解析

[可変窓を用いた高速再帰スペクトル解析　中辻 秀人　著][ref1]で解説されているアルゴリズムのcpp実装
(著者とは無関係です。)

## 説明

詳しい説明は、本を読んでください。
複数の、異なる周波数のT周期分の正弦波および余弦波を用いて、与えられた信号との内積を計算することにより、信号をスペクトル分解します。

## 使い方

```shell
make
./fmrs test.wav
```
test_spec.binとtest_aasc.binが生成されます。
ここで、_spec.binは各設定ユニットで検出されたスペクトル、_aasc.binは実分解波が出力されています。
float(32bit)でユニット数xサンプル数だけバイナリで出力しています。

## 免責

巻末のコードがBasicだったり微妙に誤植があったりしたので、自分で使うためにcで書き直しました。
多分あってる気がしますが間違えていても保証できないのでご自分で確認ください。

## License
このコード自体はCC0で公開します。
These codes are licensed under CC0.

[![CC0](http://i.creativecommons.org/p/zero/1.0/88x31.png "CC0")](http://creativecommons.org/publicdomain/zero/1.0/deed.en)

アルゴリズムに関して、著者から別途の指示がある場合はそれに従ってください。その他特許等のライセンスに関しては関知しません。

[ref1]:https://www.amazon.co.jp/dp/4862238378
