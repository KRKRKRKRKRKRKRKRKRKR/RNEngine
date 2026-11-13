#include "Game/GameApp.h"
#include "Engine/Engine.h"
#include "imgui.h"


void GameApp::Init(Engine& engine)
{
	texture0 = engine.LoadTexture(L"Resource/UVChecker0.png");	//画像(番号を受け取る)
	texture1 = engine.LoadTexture(L"Resource/UVChecker1.png");

	for(int i = 0; i < 10; i++)
	{
		m_sprite = engine.CreateSprite(texture0, { 128.0f+ i*30, 128.0f + i*30 }, { 256.0f, 256.0f });
	}
	Camera& camera = engine.GetCamera();
	camera.transform.position = { 2.0f,2.0f,-2.0f };
	camera.transform.rotation = { 35.0f,-45.0f,0.0f };
	camera.fovY = 60;
}

void GameApp::Update(Engine& engine, float dt)
{
	//ゲーム全体のルール(GameObjectのUpdateの後に呼ばれる)
}

void GameApp::Draw(Engine& engine)
{
	//その場で描く分(自動描画のModelは書かなくてよい)
	engine.DrawLine({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });	//x軸 赤
	engine.DrawLine({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f });	//y軸 緑
	engine.DrawLine({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f });	//z軸 青

	engine.DrawGrid();														//床(10x10、1マス1)
}

void GameApp::DrawUI(Engine& engine)
{

	const Input& input = engine.GetInput();
	ImGui::Begin("Mouse");										//マウス入力の確認
	ImGui::Text("Position %.0f, %.0f", input.GetMousePosition().x, input.GetMousePosition().y);
	ImGui::Text("Delta    %.0f, %.0f", input.GetMouseDelta().x, input.GetMouseDelta().y);
	ImGui::Text("Wheel    %.1f", input.GetWheel());
	ImGui::Text("L %d  R %d  M %d", input.IsDown(VK_LBUTTON), input.IsDown(VK_RBUTTON), input.IsDown(VK_MBUTTON));
	ImGui::End();

	Camera& camera = engine.GetCamera();
	ImGui::Begin("Camera");										//ゲームのカメラ
	ImGui::DragFloat3("Position", &camera.transform.position.x, 0.01f);
	ImGui::DragFloat3("Rotation", &camera.transform.rotation.x, 0.5f);	//度(x=見下ろす、y=左右を向く)
	ImGui::DragFloat("FovY", &camera.fovY, 0.5f, 10.0f, 120.0f);
	bool follow = camera.transform.parent != nullptr;

	ImGui::End();

	DebugCamera& debugCamera = engine.GetDebugCamera();

}
