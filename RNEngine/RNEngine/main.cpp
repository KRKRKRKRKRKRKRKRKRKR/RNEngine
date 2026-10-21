#include "Common.h"
#include "OS/Window.h"
#include "GPU/GraphicsDevice.h"

const UINT WINDOW_WIDTH = 1280;				//描画幅
const UINT WINDOW_HEIGHT = 720;				//描画高さ
const UINT FRAME_COUNT = 2;					//バックバッファ数

HWND g_hwnd = nullptr;						//仮:SwapChain分離まで

ComPtr<IDXGISwapChain3> g_swapChain;					//画面バッファ管理
UINT g_frameIndex = 0;									//今描くバッファ番号
ComPtr<ID3D12DescriptorHeap> g_rtvHeap;					//RTVの配列
UINT g_rtvDescriptorSize = 0;							//RTV1個のサイズ
ComPtr<ID3D12Resource> g_renderTargets[FRAME_COUNT];	//バックバッファ

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

//SwapChainとRTVの初期化
void InitSwapChain(GraphicsDevice& gfx)
{
	//SwapChain設定
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = WINDOW_WIDTH;
	scDesc.Height = WINDOW_HEIGHT;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;			//RGBA各8bit
	scDesc.SampleDesc.Count = 1;							//MSAAなし
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//描画先として使用
	scDesc.BufferCount = FRAME_COUNT;						//表と裏
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		//DX12推奨方式

	//SwapChain作成
	ComPtr<IDXGISwapChain1> swapChain1;
	CheckHR(gfx.GetFactory()->CreateSwapChainForHwnd(
		gfx.GetCommandQueue(),				//Deviceではなく Queue
		g_hwnd, &scDesc, nullptr, nullptr, &swapChain1));
	CheckHR(gfx.GetFactory()->MakeWindowAssociation(g_hwnd, DXGI_MWA_NO_ALT_ENTER));	//Alt+Enter無効

	//SwapChain3へ変換
	CheckHR(swapChain1.As(&g_swapChain));

	//現在のバッファ番号
	g_frameIndex = g_swapChain->GetCurrentBackBufferIndex();

	//RTVヒープ
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = FRAME_COUNT;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	CheckHR(gfx.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&g_rtvHeap)));

	//RTV1個のサイズ
	g_rtvDescriptorSize = gfx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//バッファ取得+RTV作成
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < FRAME_COUNT; i++)
	{
		CheckHR(g_swapChain->GetBuffer(i, IID_PPV_ARGS(&g_renderTargets[i])));
		gfx.GetDevice()->CreateRenderTargetView(g_renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += g_rtvDescriptorSize;	//次の枠へ
	}
}

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
void InitAssets(GraphicsDevice& gfx)
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
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;	//SwapChainと一致
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
void Render(GraphicsDevice& gfx)
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
	barrier.Transition.pResource = g_renderTargets[g_frameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	cmd->ResourceBarrier(1, &barrier);

	//今のバッファのRTV
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += g_frameIndex * g_rtvDescriptorSize;	//frameIndex番目へ

	//描画先セット+クリア
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

	CheckHR(g_swapChain->Present(1, 0));	//画面に表示 1=VSync待ち

	gfx.WaitForGPU();						//GPU完了待ち
	g_frameIndex = g_swapChain->GetCurrentBackBufferIndex();	//次のバッファ
}

//ここから始まる
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Window window;
	window.Init(hInstance, WINDOW_WIDTH, WINDOW_HEIGHT, L"RNEngine");
	g_hwnd = window.GetHwnd();				//仮:SwapChain分離まで

	GraphicsDevice gfx;
	gfx.Init();								//Device・Queue・Fence
	InitSwapChain(gfx);						//SwapChain・RTV
	InitAssets(gfx);						//PSO・頂点バッファ

	window.Show(nCmdShow);

	//メインループ
	while (window.ProcessMessages())		//falseで終了
	{
		Render(gfx);
	}

	return 0;								//gfxのデストラクタでGPU待ち+解放
}
