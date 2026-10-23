# Pipeline
どう描くか(シェーダーと描画設定)を決める。 [[クラス構成]]

[[Mesh]]=何を描くか / Pipeline=どう描くか

## 関数
- `Init(device, shaderFile, rtvFormat)`：
  1. ルートシグネチャ作成(シェーダーへの外部入力の一覧。今は空。`ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT`で頂点バッファ使用)
  2. `CompileShader`でVS/PSをコンパイル
  3. 入力レイアウト定義(POSITION=0byte目のfloat3 / COLOR=12byte目のfloat4。[[Mesh]]のVertexと一致させる)
  4. PSO作成(三角形・塗りつぶし・裏面カリング・時計回りが表・ブレンドなし・深度なし)
- `Set(cmd)`：`SetPipelineState` + `SetGraphicsRootSignature`
- `CompileShader(file, entry, target)`(static)：`D3DCompileFromFile`でコンパイル。エラーは出力ウィンドウへ。DEBUGでは最適化オフ

## シェーダー(shaders.hlsl)
- `VSMain`：頂点ごと。float3→float4(xyz,1)、色はそのまま
- `PSMain`：ピクセルごと。補間された色を出力(グラデーションは補間による)

## なぜPSOにまとめるか
DX12は描画設定を個別に変えられない。事前に1個にまとめて作り、描画時の切り替えを速くする。変えたいならPipelineを別に作る。

## 課題
- `Init`が長い → `CreateRootSignature`に分ける
- 入力レイアウトがVertexと2箇所管理
- 深度なし → 3Dの前後関係には深度バッファが必要
