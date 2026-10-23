#include "Renderer/Renderer.h"

Renderer::~Renderer()
{
	m_gfx.WaitForGPU();						//メンバ破棄前にGPU完了を待つ
}

void Renderer::Init(HWND hwnd, UINT width, UINT height)
{
	m_gfx.Init();											//Device・Queue・Fence
	m_swapChain.Init(m_gfx, hwnd, width, height);			//SwapChain・RTV

	m_viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	m_scissorRect = { 0, 0, (LONG)width, (LONG)height };
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
	cmd->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);	//深度なし
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };	//RGBA
	cmd->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}

void Renderer::Draw(const Pipeline& pipeline, const Mesh& mesh)
{
	ID3D12GraphicsCommandList* cmd = m_gfx.GetCommandList();
	pipeline.Set(cmd);						//描き方
	mesh.Draw(cmd);							//形
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
