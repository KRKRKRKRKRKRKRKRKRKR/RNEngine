#pragma once
#include "Common.h"

//前後関係を判定する距離の表
class DepthBuffer
{
public:
	static constexpr DXGI_FORMAT FORMAT = DXGI_FORMAT_D32_FLOAT;	//距離をfloatで保存

	void Init(ID3D12Device* device, UINT width, UINT height);
	void Clear(ID3D12GraphicsCommandList* cmd) const;	//全部1.0(一番奥)に

	D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const { return m_dsvHeap->GetCPUDescriptorHandleForHeapStart(); }
	DXGI_FORMAT GetFormat() const { return FORMAT; }

private:
	ComPtr<ID3D12Resource> m_buffer;			//深度の画像
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;		//DSVの置き場(1個)
};
