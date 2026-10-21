#include "Common.h"
#include "OS/Window.h"
#include "Renderer/Renderer.h"

const UINT WINDOW_WIDTH = 1280;				//描画幅
const UINT WINDOW_HEIGHT = 720;				//描画高さ

//ここから始まる
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Window window;
	window.Init(hInstance, WINDOW_WIDTH, WINDOW_HEIGHT, L"RNEngine");

	Renderer renderer;
	renderer.Init(window.GetHwnd(), WINDOW_WIDTH, WINDOW_HEIGHT);	//GPU・画面バッファ

	Pipeline pipeline;
	pipeline.Init(renderer.GetDevice(), L"shaders.hlsl", renderer.GetFormat());	//描き方

	//頂点データ(時計回り)
	const Vertex vertices[] =
	{
		{ {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },	//上 赤
		{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },	//右下 緑
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },	//左下 青
	};
	Mesh triangle;
	triangle.Init(renderer.GetDevice(), vertices, _countof(vertices));	//形

	window.Show(nCmdShow);

	//メインループ
	while (window.ProcessMessages())		//falseで終了
	{
		renderer.BeginFrame();
		renderer.Draw(pipeline, triangle);
		renderer.EndFrame();
	}

	return 0;								//破棄は宣言と逆順
}
