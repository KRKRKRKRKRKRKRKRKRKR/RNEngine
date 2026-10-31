#pragma once
#include "Common.h"
#include "GPU/GraphicsDevice.h"

//画面バッファを管理する
class SwapChain
{
public:
	static constexpr UINT FRAME_COUNT = 2;							//バックバッファ数
	static constexpr DXGI_FORMAT FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;	//RGBA各8bit

	void Init(GraphicsDevice& gfx, HWND hwnd, UINT width, UINT height);
	void Present();							//表示+次のバッファへ
	void Resize(ID3D12Device* device, UINT width, UINT height);	//バッファを作り直す(GPU待ちの後に呼ぶ)

	ID3D12Resource* GetCurrentBuffer() const { return m_renderTargets[m_frameIndex].Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTV() const;
	DXGI_FORMAT GetFormat() const { return FORMAT; }

private:
	void CreateRTVs(ID3D12Device* device);	//バッファを取得してRTVを作る(InitとResizeで共通)

	ComPtr<IDXGISwapChain3> m_swapChain;					//画面バッファ管理
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;					//RTVの配列
	UINT m_rtvDescriptorSize = 0;							//RTV1個のサイズ
	ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];	//バックバッファ
	UINT m_frameIndex = 0;									//今描くバッファ番号
};