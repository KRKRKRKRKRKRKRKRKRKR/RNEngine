#include "Common.h"
#include "OS/Window.h"

const UINT WINDOW_WIDTH = 1280;				//描画幅
const UINT WINDOW_HEIGHT = 720;				//描画高さ
const UINT FRAME_COUNT = 2;					//バックバッファ数

HWND g_hwnd = nullptr;						//仮:SwapChain分離まで

ComPtr<IDXGIFactory4> g_factory;						//DXGI工場
ComPtr<ID3D12Device> g_device;							//GPUの窓口
ComPtr<ID3D12CommandQueue> g_commandQueue;				//命令の投入口
ComPtr<IDXGISwapChain3> g_swapChain;					//画面バッファ管理
UINT g_frameIndex = 0;									//今描くバッファ番号
ComPtr<ID3D12DescriptorHeap> g_rtvHeap;					//RTVの配列
UINT g_rtvDescriptorSize = 0;							//RTV1個のサイズ
ComPtr<ID3D12Resource> g_renderTargets[FRAME_COUNT];	//バックバッファ
ComPtr<ID3D12CommandAllocator> g_commandAllocator;		//命令用メモリ
ComPtr<ID3D12GraphicsCommandList> g_commandList;		//命令を書く道具
ComPtr<ID3D12Fence> g_fence;							//GPU進捗カウンタ
UINT64 g_fenceValue = 0;								//次に待つ値
HANDLE g_fenceEvent = nullptr;							//待機用イベント

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

//GPUの仕事が終わるまでCPUを待機させる
void WaitForGPU()
{
	const UINT64 value = g_fenceValue;
	CheckHR(g_commandQueue->Signal(g_fence.Get(), value));	//終わったらvalueにして
	g_fenceValue++;

	if (g_fence->GetCompletedValue() < value)				//まだ終わってない
	{
		CheckHR(g_fence->SetEventOnCompletion(value, g_fenceEvent));	//終わったらイベント
		WaitForSingleObject(g_fenceEvent, INFINITE);					//鳴るまで待機
	}

	g_frameIndex = g_swapChain->GetCurrentBackBufferIndex();	//次のバッファ
}

//DX12の初期化
void InitD3D()
{
	//デバッグレイヤー
	UINT factoryFlags = 0;
#ifdef _DEBUG
	ComPtr<ID3D12Debug> debug;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
	{
		debug->EnableDebugLayer();			//エラーを出力に表示
		factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif

	//Factory
	CheckHR(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&g_factory)));

	//Device
	CheckHR(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&g_device)));	//nullptr=既定GPU

	//CommandQueue
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CheckHR(g_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&g_commandQueue)));

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
	CheckHR(g_factory->CreateSwapChainForHwnd(
		g_commandQueue.Get(),				//Deviceではなく Queue
		g_hwnd, &scDesc, nullptr, nullptr, &swapChain1));
	CheckHR(g_factory->MakeWindowAssociation(g_hwnd, DXGI_MWA_NO_ALT_ENTER));	//Alt+Enter無効

	//SwapChain3へ変換
	CheckHR(swapChain1.As(&g_swapChain));

	//現在のバッファ番号
	g_frameIndex = g_swapChain->GetCurrentBackBufferIndex();

	//RTVヒープ
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = FRAME_COUNT;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	CheckHR(g_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&g_rtvHeap)));

	//RTV1個のサイズ
	g_rtvDescriptorSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//バッファ取得+RTV作成
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < FRAME_COUNT; i++)
	{
		CheckHR(g_swapChain->GetBuffer(i, IID_PPV_ARGS(&g_renderTargets[i])));
		g_device->CreateRenderTargetView(g_renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += g_rtvDescriptorSize;	//次の枠へ
	}

	//CommandAllocator
	CheckHR(g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_commandAllocator)));

	//CommandList
	CheckHR(g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		g_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&g_commandList)));

	//すぐ閉じる
	CheckHR(g_commandList->Close());

	//Fence
	CheckHR(g_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)));
	g_fenceValue = 1;

	//Event
	g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (g_fenceEvent == nullptr)
	{
		CheckHR(HRESULT_FROM_WIN32(GetLastError()));
	}

	//動作確認として1回待つ
	WaitForGPU();
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
void InitAssets()
{
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
	CheckHR(g_device->CreateRootSignature(0, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(),
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

	CheckHR(g_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&g_pipelineState)));

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

	CheckHR(g_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
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
void Render()
{
	CheckHR(g_commandAllocator->Reset());	//前フレームの命令を破棄
	CheckHR(g_commandList->Reset(g_commandAllocator.Get(), g_pipelineState.Get()));
	g_commandList->SetGraphicsRootSignature(g_rootSignature.Get());
	g_commandList->RSSetViewports(1, &g_viewport);
	g_commandList->RSSetScissorRects(1, &g_scissorRect);

	//バリア 表示用→描画先
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = g_renderTargets[g_frameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	g_commandList->ResourceBarrier(1, &barrier);

	//今のバッファのRTV
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += g_frameIndex * g_rtvDescriptorSize;	//frameIndex番目へ

	//描画先セット+クリア
	g_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);	//深度なし
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };			//RGBA
	g_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	//三角形を描画
	g_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	//3頂点で1三角形
	g_commandList->IASetVertexBuffers(0, 1, &g_vertexBufferView);
	g_commandList->DrawInstanced(3, 1, 0, 0);				//3頂点1個

	//バリア 描画先→表示用
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	g_commandList->ResourceBarrier(1, &barrier);

	//記録終了
	CheckHR(g_commandList->Close());

	//GPUへ送信
	ID3D12CommandList* lists[] = { g_commandList.Get() };
	g_commandQueue->ExecuteCommandLists(1, lists);

	//画面に表示
	CheckHR(g_swapChain->Present(1, 0));	//1=VSync待ち

	//GPU完了待ち
	WaitForGPU();
}

//ここから始まる
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Window window;
	window.Init(hInstance, WINDOW_WIDTH, WINDOW_HEIGHT, L"RNEngine");
	g_hwnd = window.GetHwnd();				//仮:SwapChain分離まで

	InitD3D();								//DX12初期化(hwnd作成後)
	InitAssets();

	window.Show(nCmdShow);

	//メインループ
	while (window.ProcessMessages())		//falseで終了
	{
		Render();
	}

	//終了処理
	WaitForGPU();							//GPU完了後に破棄
	CloseHandle(g_fenceEvent);

	return 0;
}
