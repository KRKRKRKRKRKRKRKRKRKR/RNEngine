//画面に表示されているバッファ､を管理し､描画先として使用するためのクラス
//片方を描画する間に､もう片方を表示することで､ちらつきのない描画を実現する

#include "GPU/SwapChain.h"

//初期設定
void SwapChain::Init(GraphicsDevice& gfx, HWND hwnd, UINT width, UINT height)
{
	//SwapChain設定
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = width;
	scDesc.Height = height;
	scDesc.Format = FORMAT;
	scDesc.SampleDesc.Count = 1;							//MSAAなし
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//描画先として使用
	scDesc.BufferCount = FRAME_COUNT;						//表と裏
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		//DX12推奨方式

	//SwapChain作成
	ComPtr<IDXGISwapChain1> swapChain1;
	CheckHR(gfx.GetFactory()->CreateSwapChainForHwnd(
		gfx.GetCommandQueue(),				//Deviceではなく Queue
		hwnd, &scDesc, nullptr, nullptr, &swapChain1));
	CheckHR(gfx.GetFactory()->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));	//Alt+Enter無効

	//SwapChain3へ変換
	CheckHR(swapChain1.As(&m_swapChain));

	//現在のバッファ番号
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	//RTVヒープ
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = FRAME_COUNT;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	CheckHR(gfx.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeap)));

	//RTV1個のサイズ
	m_rtvDescriptorSize = gfx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//バッファ取得+RTV作成
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < FRAME_COUNT; i++)
	{
		CheckHR(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
		gfx.GetDevice()->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += m_rtvDescriptorSize;	//次の枠へ
	}
}

//画面に表示する
void SwapChain::Present()
{
	CheckHR(m_swapChain->Present(1, 0));	//1=VSync待ち
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();	//次のバッファ
}

//現在の描画先バッファのRTVを取得
D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::GetCurrentRTV() const
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += m_frameIndex * m_rtvDescriptorSize;	//frameIndex番目へ
	return handle;
}