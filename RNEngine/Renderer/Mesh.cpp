#include "Renderer/Mesh.h"
#include <vector>			//std::vector
#include <cmath>			//sin, cos

//CPUから書けるバッファを作り、dataを書き込む
ComPtr<ID3D12Resource> Mesh::CreateUploadBuffer(ID3D12Device* device, const void* data, UINT size)
{
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;			//CPUから書ける

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = size;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ComPtr<ID3D12Resource> buffer;
	CheckHR(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer)));

	//データを書き込み
	void* mapped = nullptr;
	D3D12_RANGE readRange = { 0, 0 };					//CPUからは読まない
	CheckHR(buffer->Map(0, &readRange, &mapped));
	memcpy(mapped, data, size);
	buffer->Unmap(0, nullptr);

	return buffer;
}

//インデックスなし
void Mesh::Init(ID3D12Device* device, const Vertex* vertices, UINT vertexCount)
{
	m_vertexCount = vertexCount;
	const UINT vbSize = sizeof(Vertex) * vertexCount;	//全体サイズ

	m_vertexBuffer = CreateUploadBuffer(device, vertices, vbSize);

	//頂点バッファビュー
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);	//1頂点のサイズ
	m_vertexBufferView.SizeInBytes = vbSize;
}

//インデックスあり
void Mesh::Init(ID3D12Device* device, const Vertex* vertices, UINT vertexCount,
	const UINT* indices, UINT indexCount)
{
	Init(device, vertices, vertexCount);				//頂点側は同じ

	m_indexCount = indexCount;
	const UINT ibSize = sizeof(UINT) * indexCount;		//全体サイズ

	m_indexBuffer = CreateUploadBuffer(device, indices, ibSize);

	//インデックスバッファビュー
	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;	//1個=32bit整数
	m_indexBufferView.SizeInBytes = ibSize;
}

void Mesh::Draw(ID3D12GraphicsCommandList* cmd) const
{
	cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	//3頂点で1三角形
	cmd->IASetVertexBuffers(0, 1, &m_vertexBufferView);

	if (m_indexCount > 0)
	{
		cmd->IASetIndexBuffer(&m_indexBufferView);
		cmd->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);	//インデックス順に描く
	}
	else
	{
		cmd->DrawInstanced(m_vertexCount, 1, 0, 0);
	}
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
		{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },	//0 左上
		{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },	//1 右上
		{ {  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },	//2 右下
		{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },	//3 左下
	};

	//三角形2枚(時計回り)
	const UINT indices[] =
	{
		0, 1, 2,	//左上→右上→右下
		0, 2, 3,	//左上→右下→左下
	};

	Init(device, vertices, _countof(vertices), indices, _countof(indices));
}

void Mesh::InitSphere(ID3D12Device* device, float radius, UINT slices, UINT stacks)
{
	const float pi = 3.14159265f;

	//頂点データ
	std::vector<Vertex> vertices;
	for (UINT i = 0; i <= stacks; i++)						//縦(上から下へ)
	{
		float theta = pi * i / stacks;						//0〜π
		for (UINT j = 0; j <= slices; j++)					//横(1周)
		{
			float phi = 2.0f * pi * j / slices;				//0〜2π

			Vertex v;
			v.position[0] = radius * std::sin(theta) * std::cos(phi);	//x
			v.position[1] = radius * std::cos(theta);					//y
			v.position[2] = radius * std::sin(theta) * std::sin(phi);	//z
			v.color[0] = 1.0f;										//白
			v.color[1] = 1.0f;
			v.color[2] = 1.0f;
			v.color[3] = 1.0f;
			v.uv[0] = (float)j / slices;							//u
			v.uv[1] = (float)i / stacks;							//v
			vertices.push_back(v);
		}
	}

	//インデックス(1マスを三角形2枚、時計回り)
	std::vector<UINT> indices;
	for (UINT i = 0; i < stacks; i++)
	{
		for (UINT j = 0; j < slices; j++)
		{
			UINT a = i * (slices + 1) + j;					//左上
			UINT b = a + 1;									//右上
			UINT c = a + (slices + 1);						//左下
			UINT d = c + 1;									//右下

			indices.push_back(a);
			indices.push_back(b);
			indices.push_back(d);

			indices.push_back(a);
			indices.push_back(d);
			indices.push_back(c);
		}
	}

	Init(device, vertices.data(), (UINT)vertices.size(), indices.data(), (UINT)indices.size());
}
