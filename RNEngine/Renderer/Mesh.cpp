#include "Renderer/Mesh.h"

void Mesh::Init(ID3D12Device* device, const Vertex* vertices, UINT vertexCount)
{
	m_vertexCount = vertexCount;
	const UINT vbSize = sizeof(Vertex) * vertexCount;	//全体サイズ

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
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer)));

	//頂点データを書き込み
	void* mapped = nullptr;
	D3D12_RANGE readRange = { 0, 0 };					//CPUからは読まない
	CheckHR(m_vertexBuffer->Map(0, &readRange, &mapped));
	memcpy(mapped, vertices, vbSize);
	m_vertexBuffer->Unmap(0, nullptr);

	//頂点バッファビュー
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);	//1頂点のサイズ
	m_vertexBufferView.SizeInBytes = vbSize;
}

void Mesh::Draw(ID3D12GraphicsCommandList* cmd) const
{
	cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	//3頂点で1三角形
	cmd->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	cmd->DrawInstanced(m_vertexCount, 1, 0, 0);
}

void Mesh::InitTriangle(ID3D12Device* device)
{
	//頂点データ(時計回り)
	const Vertex vertices[] =
	{
		{ {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.5f, 0.0f } },	//上 赤
		{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },	//右下 緑
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },	//左下 青
	};

	Init(device, vertices, _countof(vertices));
}

void Mesh::InitQuad(ID3D12Device* device)
{
	//中心が原点の1x1(ローカル座標。yは下向き)
	const Vertex vertices[] =
	{
		{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },	//左上
		{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },	//右上
		{ {  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },	//右下

		{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },	//左上
		{ {  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },	//右下
		{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },	//左下
	};
	Init(device, vertices, _countof(vertices));
}
