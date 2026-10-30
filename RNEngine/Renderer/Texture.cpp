#include "Renderer/Texture.h"
#include <DirectXTex.h>		//画像読み込み
#include <d3dx12.h>			//バリアなどの補助

void Texture::Init(GraphicsDevice& gfx, const wchar_t* file)
{
	ID3D12Device* device = gfx.GetDevice();

	//画像読み込み(CPU側)
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage image;
	Log::Info("Texture load: %ls", file);					//%lsはwchar_t文字列
	CHECK_HR(DirectX::LoadFromWICFile(file, DirectX::WIC_FLAGS_NONE, &metadata, image));

	//GPU側のテクスチャ作成(作成直後はコピー先の状態)
	CheckHR(DirectX::CreateTexture(device, metadata, &m_resource));

	//アップロード用データの準備
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	CheckHR(DirectX::PrepareUpload(device, image.GetImages(), image.GetImageCount(), metadata, subresources));

	//アップロードバッファ作成
	const UINT64 uploadSize = GetRequiredIntermediateSize(m_resource.Get(), 0, (UINT)subresources.size());
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);		//CPUから書ける
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadSize);
	ComPtr<ID3D12Resource> upload;
	CheckHR(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload)));

	//コピー命令を記録
	gfx.ResetCommandList(nullptr);
	ID3D12GraphicsCommandList* cmd = gfx.GetCommandList();
	UpdateSubresources(cmd, m_resource.Get(), upload.Get(), 0, 0, (UINT)subresources.size(), subresources.data());

	//コピー先→シェーダーで読む状態
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmd->ResourceBarrier(1, &barrier);

	gfx.ExecuteCommandList();
	gfx.WaitForGPU();										//コピー完了まで待つ(uploadを破棄する前に)

	//SRVヒープ(シェーダーから見える)
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	CheckHR(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_srvHeap)));

	//SRV(この画像をこう読む、という説明)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;	//RGBAそのまま
	srvDesc.Texture2D.MipLevels = (UINT)metadata.mipLevels;
	device->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
}

void Texture::Set(ID3D12GraphicsCommandList* cmd) const
{
	ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
	cmd->SetDescriptorHeaps(1, heaps);					//このヒープを使う
	cmd->SetGraphicsRootDescriptorTable(1, m_srvHeap->GetGPUDescriptorHandleForHeapStart());	//[1]番に渡す
}