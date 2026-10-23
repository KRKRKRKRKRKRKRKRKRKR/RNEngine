# Renderer
1フレームの描画の流れを組み立ててGPUに指示する。 [[クラス構成]]

「何を・どう描くか」は外から受け取る(`Draw(pipeline, mesh)`)。

## 持つもの
[[GraphicsDevice]] / [[SwapChain]] / Viewport / ScissorRect
- `m_gfx`を先に宣言 → 破棄は逆順なので後に破棄される

## 流れ
`BeginFrame` → `Draw`(複数可) → `EndFrame`

## 関数
- `~Renderer`：`WaitForGPU`してから破棄(GPU使用中のメンバを壊さない)
- `Init`：GraphicsDevice初期化 → SwapChain初期化 → Viewport/Scissor設定
- `BeginFrame`：
  1. `ResetCommandList`で記録開始
  2. Viewport・Scissorセット
  3. `Transition`：表示用→描画先
  4. `OMSetRenderTargets`で描画先セット
  5. `ClearRenderTargetView`で紺色クリア
- `Draw`：`pipeline.Set(cmd)`(描き方) → `mesh.Draw(cmd)`(形)
- `EndFrame`：
  1. `Transition`：描画先→表示用
  2. `ExecuteCommandList`でGPUへ送信
  3. `Present`で表示
  4. `WaitForGPU`で完了待ち
- `Transition`(static)：リソースバリア。バッファの状態を切り替える。状態が合わないとGPUが不正な状態で読み書きする
- `GetDevice` / `GetFormat`：Pipeline・Mesh初期化用に外へ渡す

## 課題
- `Renderer.cpp`が2つある(旧ファイル`RNEngine\Renderer.cpp`)。二重定義に注意
- `EndFrame`で毎フレーム`WaitForGPU` → CPUとGPUが並列に動けない
