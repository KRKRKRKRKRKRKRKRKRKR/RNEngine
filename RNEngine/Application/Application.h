#pragma once
#include "Common.h"
#include "OS/Window.h"
#include "Renderer/Renderer.h"
#include  "Math/Transform.h"
//アプリの流れ(初期化→ループ→終了)を回す
class Application
{
public:
	void Init(HINSTANCE hInstance, int nCmdShow);
	void Run();								//メインループ

private:
	void Draw();							//1フレームの描画

	static constexpr UINT WIDTH = 1280;		//描画幅
	static constexpr UINT HEIGHT = 720;		//描画高さ

	Window m_window;						//最初に宣言(最後に破棄)
	Renderer m_renderer;
	Pipeline m_pipeline;
	Mesh m_triangle;
	Transform m_triangleTransform;
};