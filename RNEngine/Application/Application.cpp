#include "Application/Application.h"
#include "imgui.h"

void Application::Init(HINSTANCE hInstance, int nCmdShow)
{
	m_window.Init(hInstance, WIDTH, HEIGHT, L"RNEngine");
	m_renderer.Init(m_window.GetHwnd(), WIDTH, HEIGHT);	//GPU・画面バッファ

	m_pipeline.Init(m_renderer.GetDevice(), L"shaders.hlsl", m_renderer.GetFormat());	//描き方

	//頂点データ(時計回り)
	const Vertex vertices[] =
	{
		{ {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },	//上 赤
		{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },	//右下 緑
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },	//左下 青
	};
	m_triangle.Init(m_renderer.GetDevice(), vertices, _countof(vertices));	//形

	m_window.Show(nCmdShow);
}

void Application::Run()
{
	while (m_window.ProcessMessages())		//falseで終了
	{
		Draw();
	}
}

void Application::Draw()
{
	m_renderer.BeginFrame();
	m_renderer.Draw(m_pipeline, m_triangle);	//三角形
	m_renderer.BeginUI();
	ImGui::ShowDemoWindow();					//動作確認用
	m_renderer.EndUI();
	m_renderer.EndFrame();
}