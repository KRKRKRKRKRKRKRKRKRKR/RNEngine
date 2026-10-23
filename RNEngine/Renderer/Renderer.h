#pragma once
#include "Common.h"
#include "GPU/GraphicsDevice.h"
#include "GPU/SwapChain.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Mesh.h"

//何をどう描くか指示する
class Renderer
{
public:
	~Renderer();

	void Init(HWND hwnd, UINT width, UINT height);
	void BeginFrame();									//記録開始+画面クリア
	void Draw(const Pipeline& pipeline, const Mesh& mesh);	//描画命令を追加
	void EndFrame();									//送信+表示+GPU待ち

	ID3D12Device* GetDevice() const { return m_gfx.GetDevice(); }
	DXGI_FORMAT GetFormat() const { return m_swapChain.GetFormat(); }

private:
	static void Transition(ID3D12GraphicsCommandList* cmd, ID3D12Resource* resource,
		D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);	//バリア

	GraphicsDevice m_gfx;						//先に宣言(後に破棄)
	SwapChain m_swapChain;
	D3D12_VIEWPORT m_viewport = {};				//描画範囲
	D3D12_RECT m_scissorRect = {};				//切り抜き範囲
};