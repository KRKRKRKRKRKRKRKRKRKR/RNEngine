#include "Application/Application.h"
#include "imgui.h"

void Application::Init(HINSTANCE hInstance, int nCmdShow)
{
	m_window.Init(hInstance, WIDTH, HEIGHT, L"RNEngine");
	m_renderer.Init(m_window.GetHwnd(), WIDTH, HEIGHT);	//GPU・画面バッファ

	m_pipeline.Init(m_renderer.GetDevice(), L"shaders.hlsl", m_renderer.GetFormat());	//描き方

	GameObject triangle;
	triangle.mesh.InitTriangle(m_renderer.GetDevice());	//三角形を作る
	m_objects.push_back(triangle);

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

	for (const GameObject& obj : m_objects)
	{
		m_renderer.Draw(m_pipeline, obj.mesh, obj.transform);	//全部描く
	}

	m_renderer.BeginUI();

	ImGui::Begin("Objects");
	for (size_t i = 0; i < m_objects.size(); i++)
	{
		GameObject& obj = m_objects[i];
		ImGui::PushID((int)i);									//同じラベルの衝突を避ける
		ImGui::Text("Object %d", (int)i);
		ImGui::DragFloat3("Position", &obj.transform.position.x, 0.01f);
		ImGui::DragFloat3("Rotation", &obj.transform.rotation.x, 0.5f);
		ImGui::DragFloat3("Scale", &obj.transform.scale.x, 0.01f);
		if (ImGui::Button("Reset"))
		{
			obj.transform = Transform();						//初期値に戻す
		}
		ImGui::PopID();
	}
	ImGui::End();

	ImGui::ShowDemoWindow();									//動作確認用

	m_renderer.EndUI();

	m_renderer.EndFrame();

}