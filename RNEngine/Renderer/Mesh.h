#pragma once
#include "Common.h"

struct Vertex								//頂点1個(Pipelineの入力レイアウトと一致)
{
	float position[3];						//xyz
	float color[4];							//rgba
	float uv[2];							//テクスチャ座標(左上が0,0)
};

//形(頂点データ)を持つ
class Mesh
{
public:
	void Init(ID3D12Device* device, const Vertex* vertices, UINT vertexCount);
	void Draw(ID3D12GraphicsCommandList* cmd) const;

	void InitTriangle(ID3D12Device* device);	//三角形を作る
	void InitQuad(ID3D12Device* device);		//四角形を作る
private:
	ComPtr<ID3D12Resource> m_vertexBuffer;				//頂点データ
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};	//頂点バッファの説明
	UINT m_vertexCount = 0;								//頂点数
};