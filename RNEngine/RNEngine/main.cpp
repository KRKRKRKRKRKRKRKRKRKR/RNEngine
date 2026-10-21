#include "Common.h"
#include "OS/Window.h"
#include "GPU/GraphicsDevice.h"
#include "GPU/SwapChain.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Mesh.h"

const UINT WINDOW_WIDTH = 1280;				//描画幅
const UINT WINDOW_HEIGHT = 720;				//描画高さ

//描画
void Render(GraphicsDevice& gfx, SwapChain& swapChain, const Pipeline& pipeline, const Mesh& mesh)
{
	gfx.ResetCommandList(nullptr);			//記録開始(PSOはPipelineでセット)
	ID3D12GraphicsCommandList* cmd = gfx.GetCommandList();

	//ビューポート・シザー
	D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, 0.0f, 1.0f };
	D3D12_RECT scissorRect = { 0, 0, (LONG)WINDOW_WIDTH, (LONG)WINDOW_HEIGHT };
	cmd->RSSetViewports(1, &viewport);
	cmd->RSSetScissorRects(1, &scissorRect);

	//バリア 表示用→描画先
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = swapChain.GetCurrentBuffer();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	cmd->ResourceBarrier(1, &barrier);

	//描画先セット+クリア
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = swapChain.GetCurrentRTV();
	cmd->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);	//深度なし
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };	//RGBA
	cmd->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	//三角形を描画
	pipeline.Set(cmd);						//描き方
	mesh.Draw(cmd);							//形

	//バリア 描画先→表示用
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	cmd->ResourceBarrier(1, &barrier);

	gfx.ExecuteCommandList();				//記録終了+GPUへ送信
	swapChain.Present();					//画面に表示+次のバッファへ
	gfx.WaitForGPU();						//GPU完了待ち
}

//ここから始まる
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Window window;
	window.Init(hInstance, WINDOW_WIDTH, WINDOW_HEIGHT, L"RNEngine");

	GraphicsDevice gfx;						//最初に宣言(最後に破棄)
	gfx.Init();								//Device・Queue・Fence

	SwapChain swapChain;
	swapChain.Init(gfx, window.GetHwnd(), WINDOW_WIDTH, WINDOW_HEIGHT);	//SwapChain・RTV

	Pipeline pipeline;
	pipeline.Init(gfx.GetDevice(), L"shaders.hlsl", swapChain.GetFormat());	//描き方

	//頂点データ(時計回り)
	const Vertex vertices[] =
	{
		{ {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },	//上 赤
		{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },	//右下 緑
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },	//左下 青
	};
	Mesh triangle;
	triangle.Init(gfx.GetDevice(), vertices, _countof(vertices));	//形

	window.Show(nCmdShow);

	//メインループ
	while (window.ProcessMessages())		//falseで終了
	{
		Render(gfx, swapChain, pipeline, triangle);
	}

	return 0;								//破棄は宣言と逆順
}
