#pragma once
#include "Common.h"
#include "GPU/GraphicsDevice.h"

//画像1枚をGPUに持つ
class Texture
{
public:
	void Init(GraphicsDevice& gfx, const wchar_t* file);	//読み込み+GPUへ転送
	void Set(ID3D12GraphicsCommandList* cmd) const;			//テクスチャをセット
private:
	ComPtr<ID3D12Resource> m_resource;						//GPU上の画像
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;					//SRV置き場
};
