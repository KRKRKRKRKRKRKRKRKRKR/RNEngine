# SwapChain
画面に表示するバッファ(表と裏)を管理し、描画先として渡す。 [[クラス構成]]

## 仕組み
- バッファ2枚(`FRAME_COUNT=2`)。片方を表示中に、もう片方(裏)へ描く
- `Present`で表と裏を入れ替える(フリップ)
- 1枚だと描きかけの絵が画面に出る

## 持つもの
| メンバ | 役割 |
|---|---|
| `m_swapChain` | DXGIの画面バッファ管理 |
| `m_renderTargets[2]` | バックバッファ |
| `m_rtvHeap` | RTVを並べる配列 |
| `m_rtvDescriptorSize` | RTV1個のサイズ |
| `m_frameIndex` | 今描いているバッファ番号 |

## 関数
- `Init(gfx, hwnd, width, height)`：
  1. 設定(サイズ・RGBA8bit・MSAAなし・`FLIP_DISCARD`)
  2. `CreateSwapChainForHwnd`(第1引数はDeviceではなく**CommandQueue**)。`MakeWindowAssociation`でAlt+Enter無効
  3. `As`で`IDXGISwapChain3`に変換
  4. `GetCurrentBackBufferIndex`で番号取得
  5. RTVヒープ作成
  6. ループ2回：`GetBuffer` → `CreateRenderTargetView` → ハンドルをサイズ分進める
- `Present`：`Present(1,0)`(1=VSync) → 次のバッファ番号を取り直す
- `GetCurrentRTV`：先頭 + `frameIndex × サイズ`
- `GetCurrentBuffer` / `GetFormat`：[[Renderer]]の`Transition`と[[Pipeline]]の`RTVFormats`で使う

## RTVとは
バッファが「描画先として使える」ことをGPUに伝える説明書。[[Mesh]]の頂点バッファビューと同じ考え方。

## 課題
- ウィンドウのリサイズ未対応(`ResizeBuffers`が必要)
- ファイル先頭に空の`//`コメントが残っている
- `Present`がVSync固定
