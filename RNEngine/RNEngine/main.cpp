#include <windows.h>
#include <d3d12.h>							//Dx12本体
#include <dxgi1_6.h>						//Factory,SwapChain
#include <d3dcompiler.h>					//シェーダーのコンパイル
#include <wrl.h>							//ComPtr

#pragma comment(lib, "d3d12.lib")			//Dx12本体
#pragma comment(lib, "dxgi.lib")			//Factory,SwapChain
#pragma comment(lib, "d3dcompiler.lib")		//シェーダーのコンパイル

using Microsoft::WRL::ComPtr;

const UINT WINDOW_WIDTH = 1280;				//描画幅
const UINT WINDOW_HEIGHT = 720;				//描画高さ
const UINT FRAME_COUNT = 2;					//バックバッファ数

HWND g_hwnd = nullptr;						//ウィンドウハンドル

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


//HRESULTが失敗のときによびだす関数
inline void CheckHR(HRESULT hr)				//失敗なら停止
{
	if (FAILED(hr))
	{
		MessageBoxA(nullptr, "HRESULT failed", "Error", MB_OK);
		__debugbreak();
	}
}

//GPUの仕事が終わるまでCPUを待機させる
void WaitForGPU()							//GPUの完了を待つ
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

//DX12の初期化関数
void InitD3D()								//DX12初期化
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

	//2-14 すぐ閉じる
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

//ウェン道への通知これから閉じると処理
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)	//メッセージ処理
{
	switch (msg)
	{
	case WM_DESTROY:						//ウィンドウ破棄
		PostQuitMessage(0);					//WM_QUITを送る
		return 0;
	}
	return DefWindowProc(hwnd, msg, wp, lp);	//既定処理
}

//ここから始まる
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//クラス登録
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;		//サイズ変更時に再描画
	wc.lpfnWndProc = WndProc;				//メッセージ処理関数
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);	//矢印カーソル
	wc.lpszClassName = L"RNEngineWindow";
	RegisterClassEx(&wc);

	//サイズ計算
	RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);	//枠分を加算

	//作成・表示
	g_hwnd = CreateWindow(
		wc.lpszClassName,
		L"RNEngine",						//タイトル
		WS_OVERLAPPEDWINDOW,				//通常ウィンドウ
		CW_USEDEFAULT, CW_USEDEFAULT,		//位置はOS任せ
		rect.right - rect.left,				//幅(枠込み)
		rect.bottom - rect.top,				//高さ(枠込み)
		nullptr, nullptr, hInstance, nullptr);

	InitD3D();								//DX12初期化(hwnd作成後)

	ShowWindow(g_hwnd, nCmdShow);

	//メッセージループ
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))	//メッセージあり
		{
			TranslateMessage(&msg);			//キー入力変換
			DispatchMessage(&msg);			//WndProcへ送る
		}
		else
		{
			//ここで毎フレーム描画(Phase3以降)
		}
	}

	//6 終了処理
	WaitForGPU();							//GPU完了後に破棄
	CloseHandle(g_fenceEvent);

	return 0;
}
