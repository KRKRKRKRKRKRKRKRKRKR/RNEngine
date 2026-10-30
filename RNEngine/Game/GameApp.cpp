#include "Game/GameApp.h"
#include "Engine/Engine.h"
#include "Game/Spinner.h"
#include "imgui.h"

//Modelの値をいじるImGui窓(このcpp内だけ)
static void EditModel(const char* name, Model& model)
{
	ImGui::Begin(name);
	ImGui::DragFloat3("Position", &model.transform.position.x, 0.01f);
	ImGui::DragFloat3("Rotation", &model.transform.rotation.x, 0.5f);
	ImGui::DragFloat3("Scale", &model.transform.scale.x, 0.01f);
	ImGui::ColorEdit4("Color", &model.color.x);				//RGBA
	ImGui::Checkbox("Visible", &model.visible);
	ImGui::End();
}

void GameApp::Init(Engine& engine)
{
	texture0 = engine.LoadTexture(L"Resource/UVChecker0.png");	//画像(番号を受け取る)
	texture1 = engine.LoadTexture(L"Resource/UVChecker1.png");

	m_triangle0 = engine.CreateTriangle(texture0);				//作るだけ。描画は自動

	m_triangle1 = engine.CreateTriangle(texture1);
	m_triangle1->transform.position.x = 0.5f;
	m_triangle1->color = { 1.0f, 0.5f, 0.5f, 1.0f };			//赤みを掛ける

	m_sphere = engine.CreateSphere(texture0);
	m_sphere->transform.scale = { 0.5f, 0.5f, 0.5f };			//小さくして画面に収める

	m_sprite = engine.CreateSprite(texture0, { 640.0f, 360.0f }, { 256.0f, 256.0f });

	m_spinner = engine.Spawn<Spinner>(texture1, -0.6f, 0.3f);	//出すだけ。動きと描画は自動
}

void GameApp::DrawUI(Engine& engine)
{
	EditModel("Triangle0", *m_triangle0);
	EditModel("Triangle1", *m_triangle1);
	EditModel("Sphere", *m_sphere);

	ImGui::Begin("Sprite");
	ImGui::DragFloat2("Position", &m_sprite->position.x, 1.0f);	//ピクセル単位
	ImGui::DragFloat2("Size", &m_sprite->size.x, 1.0f);
	ImGui::DragFloat("Rotation", &m_sprite->rotation, 0.5f);	//度
	ImGui::ColorEdit4("Color", &m_sprite->color.x);
	ImGui::Checkbox("Visible", &m_sprite->visible);
	ImGui::End();

	ImGui::Begin("Spinner");									//Spawn/Destroyの確認
	if (ImGui::Button("Spawn") && !m_spinner)
	{
		m_spinner = engine.Spawn<Spinner>(texture1, -0.6f, 0.3f);
	}
	if (ImGui::Button("Destroy") && m_spinner)
	{
		m_spinner->Destroy();									//フレームの終わりに消える
		m_spinner = nullptr;									//以後使わない
	}
	ImGui::Text(m_spinner ? "alive" : "none");
	ImGui::End();

	ImGui::ShowDemoWindow();									//動作確認用
}
