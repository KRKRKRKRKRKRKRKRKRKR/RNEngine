# Mesh
描く形(頂点データ)をGPUに持たせ、描画命令を出す。 [[クラス構成]]

## Vertex
頂点1個：位置xyz + 色rgba。[[Pipeline]]の入力レイアウトと一致させる。

## 関数
- `Init(device, vertices, vertexCount)`：
  1. サイズ計算：`sizeof(Vertex) * vertexCount`
  2. `CreateCommittedResource`で頂点バッファ作成(`UPLOAD`ヒープ=CPUから書ける、初期状態`GENERIC_READ`)
  3. `Map` → `memcpy` → `Unmap`で書き込み(`readRange={0,0}`=CPUからは読まない)
  4. 頂点バッファビュー作成(先頭アドレス・1頂点のサイズ・全体サイズ)
- `Draw(cmd)`：
  1. `IASetPrimitiveTopology`：3頂点で1三角形
  2. `IASetVertexBuffers`：ビューをセット
  3. `DrawInstanced`：描画

## なぜビューが必要か
GPUはバッファをただのバイト列としか知らない。1頂点のサイズと全体サイズを教えないと区切りが分からない。

## 課題
- `UPLOAD`ヒープのまま → 頂点が増えたら`DEFAULT`ヒープへコピーする方が速い
- インデックスバッファなし → 四角形などで必要
