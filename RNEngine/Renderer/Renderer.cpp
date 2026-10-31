#include "Renderer/Renderer.h"

Renderer::~Renderer()
{
	m_gfx.WaitForGPU();						//メンバ破棄前にGPU完了を待つ
}

void Renderer::Init(HWND hwnd, UINT width, UINT height)
{
	m_gfx.Init();											//Device・Queue・Fence
	m_swapChain.Init(m_gfx, hwnd, width, height);			//SwapChain・RTV
	m_depthBuffer.Init(m_gfx.GetDevice(), width, height);	//画面と同じサイズ

	m_viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	m_aspect = (float)width / (float)height;	//アスペクト比
	m_scissorRect = { 0, 0, (LONG)width, (LONG)height };
	m_imgui.Init(hwnd, m_gfx, m_swapChain.GetFormat(), SwapChain::FRAME_COUNT);	//UI
	//平行投影行列(左上が(0,0)、右下が(幅,高さ))
	DirectX::XMStoreFloat4x4(&m_orthoProjection, DirectX::XMMatrixOrthographicOffCenterLH(0.0f, (float)width, (float)height, 0.0f, 0.0f, 1.0f));

	//線の頂点バッファ(CPUから書けるヒープ。最大数分を最初に1回だけ作る)
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeof(Vertex) * MAX_LINE_VERTICES;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	CHECK_HR(m_gfx.GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_lineBuffer)));
	D3D12_RANGE readRange = { 0, 0 };						//CPUからは読まない
	CHECK_HR(m_lineBuffer->Map(0, &readRange, (void**)&m_lineMapped));	//Unmapせず毎フレーム書く

}

void Renderer::BeginFrame()
{
	m_gfx.ResetCommandList(nullptr);		//記録開始
	ID3D12GraphicsCommandList* cmd = m_gfx.GetCommandList();

	cmd->RSSetViewports(1, &m_viewport);
	cmd->RSSetScissorRects(1, &m_scissorRect);

	Transition(cmd, m_swapChain.GetCurrentBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);	//表示用→描画先

	//描画先セット+クリア
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swapChain.GetCurrentRTV();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_depthBuffer.GetDSV();
	cmd->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);	//色+深度
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };	//RGBA
	cmd->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_depthBuffer.Clear(cmd);				//距離を全部リセット
}

//シェーダーに送る定数(shaders.hlslのcbufferと一致させる)
struct ObjectConstants
{
	DirectX::XMFLOAT4X4 world;				//ワールド行列
	Vector4 color;							//RGBA
};

void Renderer::Draw(const Pipeline& pipeline, const Mesh& mesh, const Texture& texture,
	const Transform& transform, const Vector4& color)
{
	ID3D12GraphicsCommandList* cmd = m_gfx.GetCommandList();
	pipeline.Set(cmd);						//描き方(先にルートシグネチャを設定)
	texture.Set(cmd);						//画像

	ObjectConstants constants;
	DirectX::XMMATRIX viewProj = m_camera.GetViewProjectionMatrix(m_aspect);	//カメラの行列
	DirectX::XMStoreFloat4x4(&constants.world, transform.GetWorldMatrix() * viewProj);	//ワールド→画面
	constants.color = color;
	cmd->SetGraphicsRoot32BitConstants(0, 20, &constants, 0);				//b0へ20個(行列16+色4)

	mesh.Draw(cmd);							//形

}

void Renderer::DrawSprite(const Pipeline& pipeline, const Mesh& mesh, const Texture& texture,
	const Transform& transform, const Vector4& color)
{
	ID3D12GraphicsCommandList* cmd = m_gfx.GetCommandList();
	pipeline.Set(cmd);						//描き方(先にルートシグネチャを設定)
	texture.Set(cmd);						//画像

	DirectX::XMMATRIX projection = DirectX::XMLoadFloat4x4(&m_orthoProjection);	//保存用→計算用

	ObjectConstants constants;
	DirectX::XMStoreFloat4x4(&constants.world, transform.GetWorldMatrix() * projection);	//ワールド→画面
	constants.color = color;
	cmd->SetGraphicsRoot32BitConstants(0, 20, &constants, 0);				//b0へ20個(行列16+色4)

	mesh.Draw(cmd);							//形
}

void Renderer::DrawLines(const Pipeline& pipeline, const std::vector<Vertex>& vertices)
{
	if (vertices.empty()) return;

	UINT count = (UINT)vertices.size();
	if (count > MAX_LINE_VERTICES)
	{
		Log::Warn("Too many lines: %u vertices (max %u)", count, MAX_LINE_VERTICES);
		count = MAX_LINE_VERTICES;			//入る分だけ描く
	}
	memcpy(m_lineMapped, vertices.data(), sizeof(Vertex) * count);	//EndFrameでGPUを待っているので上書きしてよい

	ID3D12GraphicsCommandList* cmd = m_gfx.GetCommandList();
	pipeline.Set(cmd);

	ObjectConstants constants;
	DirectX::XMStoreFloat4x4(&constants.world, m_camera.GetViewProjectionMatrix(m_aspect));	//頂点は既にワールド座標
	constants.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	cmd->SetGraphicsRoot32BitConstants(0, 20, &constants, 0);

	D3D12_VERTEX_BUFFER_VIEW view = {};
	view.BufferLocation = m_lineBuffer->GetGPUVirtualAddress();
	view.StrideInBytes = sizeof(Vertex);
	view.SizeInBytes = sizeof(Vertex) * count;
	cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);	//2頂点で1本
	cmd->IASetVertexBuffers(0, 1, &view);
	cmd->DrawInstanced(count, 1, 0, 0);		//全部を1回で
}



void Renderer::EndFrame()
{
	Transition(m_gfx.GetCommandList(), m_swapChain.GetCurrentBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);	//描画先→表示用

	m_gfx.ExecuteCommandList();				//記録終了+GPUへ送信
	m_swapChain.Present();					//画面に表示+次のバッファへ
	m_gfx.WaitForGPU();						//GPU完了待ち
}

void Renderer::Transition(ID3D12GraphicsCommandList* cmd, ID3D12Resource* resource,
	D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = resource;
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	cmd->ResourceBarrier(1, &barrier);
}
void Renderer::BeginUI()
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swapChain.GetCurrentRTV();
	m_gfx.GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);	//UIは深度なし
	m_imgui.BeginFrame();
}

void Renderer::EndUI()
{
	m_imgui.Render(m_gfx.GetCommandList());
}
