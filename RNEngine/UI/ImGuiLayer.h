#pragma once
#include "Common.h"
#include "GPU/GraphicsDevice.h"

//ImGuiの初期化・フレーム処理・終了を行う
class ImGuiLayer
{
public:
	~ImGuiLayer();

	void Init(HWND hwnd, GraphicsDevice& gfx, DXGI_FORMAT rtvFormat, UINT frameCount);
	void BeginFrame();									//UI記録開始
	void Render(ID3D12GraphicsCommandList* cmd);		//UI描画命令を追加

private:
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;				//フォント用SRV
	bool m_initialized = false;							//Init済みか
};