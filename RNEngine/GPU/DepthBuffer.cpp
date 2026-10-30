#include "GPU/DepthBuffer.h"

void DepthBuffer::Init(ID3D12Device* device, UINT width, UINT height)
{
	//深度の画像
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;		//GPU専用メモリ

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = width;								//画面と同じサイズ
	desc.Height = height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = FORMAT;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;		//並びはGPU任せ
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	//深度として使う

	D3D12_CLEAR_VALUE clearValue = {};				//最適なクリア値(高速化のヒント)
	clearValue.Format = FORMAT;
	clearValue.DepthStencil.Depth = 1.0f;

	CheckHR(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&m_buffer)));	//最初から書き込み状態

	//DSVヒープ
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	CheckHR(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_dsvHeap)));

	//DSV作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = FORMAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(m_buffer.Get(), &dsvDesc, GetDSV());
}

void DepthBuffer::Clear(ID3D12GraphicsCommandList* cmd) const
{
	cmd->ClearDepthStencilView(GetDSV(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);	//1.0=一番奥
}
