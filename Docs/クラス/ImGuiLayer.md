# ImGuiLayer
ImGuiの初期化・フレーム処理・描画・終了を行う、エンジンとImGuiの接続層。 [[クラス構成]]

## なぜ`ImGui`ではなく`ImGuiLayer`か
- `ImGui::`は既にImGuiの名前空間。`class ImGui`だと衝突する
- ImGui本体ではなく、エンジンへ組み込むための薄い層という役割を名前で表す

## 持つもの
- `m_srvHeap`：フォントテクスチャ用のSRVヒープ(1個、シェーダーから見える)
- `m_initialized`：Init済みか(デストラクタで後始末するか判断)

## 関数
- `Init(hwnd, gfx, rtvFormat, frameCount)`：
  1. SRVヒープ作成
  2. `CreateContext`、ダークテーマ、`DockingEnable`
  3. `ImGui_ImplWin32_Init`(入力はWin32から受け取る)
  4. `ImGui_ImplDX12_InitInfo`を設定して`ImGui_ImplDX12_Init`(SRVのAlloc/Freeはラムダで1個だけ使う)
- `BeginFrame`：DX12とWin32の`NewFrame` → `ImGui::NewFrame` → `DockSpaceOverViewport`(画面全体をドッキング領域に)
- `Render(cmd)`：`ImGui::Render` → `SetDescriptorHeaps` → `RenderDrawData`
- `~ImGuiLayer`：DX12とWin32の`Shutdown` → `DestroyContext`

## 流れ(Applicationから)
`BeginFrame` → `Draw`(三角形) → `BeginUI` → ImGuiの呼び出し → `EndUI` → `EndFrame`
- UIは三角形の後に描く(重ねるため)
- `EndFrame`の前に命令を積む(`EndFrame`で描画先を表示用に戻すため)

## 注意
- `DockSpaceOverViewport`に`ImGuiDockNodeFlags_PassthruCentralNode`が無いと、中央が不透明になって三角形とクリア色が隠れる
- `Renderer`のメンバで最後に宣言する(最初に破棄 → GraphicsDeviceより先にImGuiが後始末される)
- `Window::WndProc`の先頭で`ImGui_ImplWin32_WndProcHandler`を呼ぶ(入力を渡す。宣言は`#if 0`の中なので自分で`extern`を書く)
- 日本語は標準フォントに無いので、ラベルは英語にする(日本語は後日フォント読み込み)

## 課題
- SRVが1個だけ → 画像表示などで増える場合はアロケーターに差し替え
- Multi-Viewports(ウィンドウを外に出す)は未対応
