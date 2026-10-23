# GraphicsDevice
GPUとの窓口を持ち、命令を送って完了を待つ。 [[クラス構成]]

## 持つもの
| 部品 | 役割 |
|---|---|
| Factory | DXGI工場。SwapChain作成に必要 |
| Device | GPUの窓口。リソース作成 |
| CommandQueue | 命令の投入口 |
| CommandAllocator | 命令用メモリ |
| CommandList | 命令を記録する道具 |
| Fence / Event | GPUの進捗を知って待つ仕組み |

## 関数
- `Init`：デバッグレイヤー(DEBUGのみ) → Factory → Device → Queue → Allocator → List(作成直後は記録中なので`Close`) → Fence → Event
- `ResetCommandList(pso)`：前フレームの命令を破棄して記録開始
- `ExecuteCommandList`：記録を閉じてQueueへ送信
- `WaitForGPU`：`Signal`で「終わったらvalueにして」と頼み、未完了ならイベントが鳴るまで待つ。CPUがGPUより先に進んで使用中メモリを書き換えるのを防ぐ
- `~GraphicsDevice`：`WaitForGPU`してから破棄

## 他クラスとの分担
GPUとのやり取りだけ。画面バッファは[[SwapChain]]、描画の流れは[[Renderer]]。

## 課題
- Allocatorが1個 → フレームを重ねる段階でフレーム分に増やす
