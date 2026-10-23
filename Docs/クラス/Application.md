# Application
部品を持ち、アプリの流れ(初期化→ループ→終了)を回す。 [[クラス構成]]

## 持つもの
[[Window]] / [[Renderer]] / [[Pipeline]] / [[Mesh]](三角形)

## 関数
- `Init`：Window作成 → Renderer初期化 → Pipeline初期化 → 頂点データ作成してMeshへ → 表示
- `Run`：`ProcessMessages()`がfalseになるまで`Draw()`を繰り返す
- `Draw`：`BeginFrame` → `Draw(pipeline, mesh)` → `EndFrame`
- 終了：`main.cpp`のデストラクタでメンバが宣言の逆順に破棄(Windowは最初に宣言)

## main.cpp との分担
`main.cpp`は`Init`→`Run`を呼ぶだけ。知識はすべてApplicationにある。

## 課題
- 頂点データ直書き → シーンが増えたらScene/Gameに切り出す
- deltaTimeなし → 移動・物理の前に必要
