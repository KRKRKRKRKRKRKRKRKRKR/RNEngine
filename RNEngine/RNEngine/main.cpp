#include "Common.h"
#include "OS/Window.h"
#include "GPU/GraphicsDevice.h"
#include "GPU/SwapChain.h"

const UINT WINDOW_WIDTH = 1280;				//描画幅
const UINT WINDOW_HEIGHT = 720;				//描画高さ

ComPtr<ID3D12RootSignature> g_rootSignature;			//シェーダー入力の一覧
ComPtr<ID3D12PipelineState> g_pipelineState;			//描画設定一式
ComPtr<ID3D12Resource> g_vertexBuffer;					//頂点データ
D3D12_VERTEX_BUFFER_VIEW g_vertexBufferView = {};		//頂点バッファの説明
D3D12_VIEWPORT g_viewport = {};							//描画範囲
D3D12_RECT g_scissorRect = {};							//切り抜き範囲

struct Vertex								//頂点1個
{
	float position[3];						//xyz
	float color[4];							//rgba
};

//シェーダーをコンパイル
ComPtr<ID3DBlob> CompileShader(const wchar_t* file, const char* entry, const char* target)
{
	UINT flags = 0;
#ifdef _DEBUG
	flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;	//デバッグ用
#endif
	ComPtr<ID3DBlob> blob, error;
	HRESULT hr = D3DCompileFromFile(file, nullptr, nullptr, entry, target, flags, 0, &blob, &error);
	if (error)
	{
		OutputDebugStringA((const char*)error->GetBufferPointer());	//HLSLエラー表示
	}
	CheckHR(hr);
	return blob;
}

//描画に使う資源の初期化
void InitAssets(GraphicsDevice& gfx, DXGI_FORMAT rtvFormat)
{
	ID3D12Device* device = gfx.GetDevice();

	//ルートシグネチャ(空)
	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;	//頂点バッファ使用
	ComPtr<ID3DBlob> rsBlob, rsError;
	HRESULT hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rsBlob, &rsError);
	if (rsError)
	{
		OutputDebugStringA((const char*)rsError->GetBufferPointer());
	}
	CheckHR(hr);
	CheckHR(device->CreateRootSignature(0, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(),
		IID_PPV_ARGS(&g_rootSignature)));

	//シェーダー
	ComPtr<ID3DBlob> vs = CompileShader(L"shaders.hlsl", "VSMain", "vs_5_0");
	ComPtr<ID3DBlob> ps = CompileShader(L"shaders.hlsl", "PSMain", "ps_5_0");

	//入力レイアウト(Vertexの中身の説明)
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	//PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = g_rootSignature.Get();
	psoDesc.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
	psoDesc.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
	psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = rtvFormat;					//SwapChainと一致
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleMask = UINT_MAX;

	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;		//塗りつぶし
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;		//裏面を描かない
	psoDesc.RasterizerState.FrontCounterClockwise = FALSE;			//時計回りが表
	psoDesc.RasterizerState.DepthClipEnable = TRUE;

	D3D12_RENDER_TARGET_BLEND_DESC& blend = psoDesc.BlendState.RenderTarget[0];
	blend.BlendEnable = FALSE;						//ブレンドなし
	blend.SrcBlend = D3D12_BLEND_ONE;
	blend.DestBlend = D3D12_BLEND_ZERO;
	blend.BlendOp = D3D12_BLEND_OP_ADD;
	blend.SrcBlendAlpha = D3D12_BLEND_ONE;
	blend.DestBlendAlpha = D3D12_BLEND_ZERO;
	blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blend.LogicOp = D3D12_LOGIC_OP_NOOP;
	blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	//0だと何も描かれない

	psoDesc.DepthStencilState.DepthEnable = FALSE;		//深度なし
	psoDesc.DepthStencilState.StencilEnable = FALSE;

	CheckHR(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&g_pipelineState)));

	//頂点データ(時計回り)
	Vertex vertices[] =
	{
		{ {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },	//上 赤
		{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },	//右下 緑
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },	//左下 青
	};
	const UINT vbSize = sizeof(vertices);

	//頂点バッファ作成
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;			//CPUから書ける

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = vbSize;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	CheckHR(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&g_vertexBuffer)));

	//頂点データを書き込み
	void* mapped = nullptr;
	D3D12_RANGE readRange = { 0, 0 };					//CPUからは読まない
	CheckHR(g_vertexBuffer->Map(0, &readRange, &mapped));
	memcpy(mapped, vertices, vbSize);
	g_vertexBuffer->Unmap(0, nullptr);

	//頂点バッファビュー
	g_vertexBufferView.BufferLocation = g_vertexBuffer->GetGPUVirtualAddress();
	g_vertexBufferView.StrideInBytes = sizeof(Vertex);	//1頂点のサイズ
	g_vertexBufferView.SizeInBytes = vbSize;			//全体サイズ

	//ビューポート・シザー
	g_viewport = { 0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, 0.0f, 1.0f };
	g_scissorRect = { 0, 0, (LONG)WINDOW_WIDTH, (LONG)WINDOW_HEIGHT };
}

//描画
void Render(GraphicsDevice& gfx, SwapChain& swapChain)
{
	gfx.ResetCommandList(g_pipelineState.Get());		//記録開始
	ID3D12GraphicsCommandList* cmd = gfx.GetCommandList();

	cmd->SetGraphicsRootSignature(g_rootSignature.Get());
	cmd->RSSetViewports(1, &g_viewport);
	cmd->RSSetScissorRects(1, &g_scissorRect);

	//バリア 表示用→描画先
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = swapChain.GetCurrentBuffer();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	cmd->ResourceBarrier(1, &barrier);

	//描画先セット+クリア
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = swapChain.GetCurrentRTV();
	cmd->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);	//深度なし
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };	//RGBA
	cmd->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	//三角形を描画
	cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	//3頂点で1三角形
	cmd->IASetVertexBuffers(0, 1, &g_vertexBufferView);
	cmd->DrawInstanced(3, 1, 0, 0);			//3頂点1個

	//バリア 描画先→表示用
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	cmd->ResourceBarrier(1, &barrier);

	gfx.ExecuteCommandList();				//記録終了+GPUへ送信
	swapChain.Present();					//画面に表示+次のバッファへ
	gfx.WaitForGPU();						//GPU完了待ち
}

//ここから始まる
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Window window;
	window.Init(hInstance, WINDOW_WIDTH, WINDOW_HEIGHT, L"RNEngine");

	GraphicsDevice gfx;						//swapChainより先に宣言(後に破棄)
	gfx.Init();								//Device・Queue・Fence

	SwapChain swapChain;
	swapChain.Init(gfx, window.GetHwnd(), WINDOW_WIDTH, WINDOW_HEIGHT);	//SwapChain・RTV

	InitAssets(gfx, swapChain.GetFormat());	//PSO・頂点バッファ

	window.Show(nCmdShow);

	//メインループ
	while (window.ProcessMessages())		//falseで終了
	{
		Render(gfx, swapChain);
	}

	return 0;								//破棄は宣言と逆順(swapChain→gfx→window)
}
