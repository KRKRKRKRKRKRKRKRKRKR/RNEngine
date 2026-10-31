#include "Game/GameApp.h"
#include "Engine/Engine.h"
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

	int beetleMesh = engine.LoadMesh("Resource/Designersoup Low Poly Car Pack Volume 1 updated/Beatall.fbx", 0.01f);	//cm→m(長さ約3.45)
	int beetleTexture = engine.LoadTexture(L"Resource/Designersoup Low Poly Car Pack Volume 1 updated/Beatall.fbm/387359c5580f06c08c266126b3b46db47e48ba44.png");	//色見本の画像
	m_beetle = engine.CreateModel(beetleMesh, beetleTexture);
	m_beetle->transform.position = { 0.6f, -0.6f, 0.0f };
	m_beetle->transform.rotation = { 15.0f, 30.0f, 0.0f };		//斜めにして立体に見せる
	m_beetle->transform.scale = { 0.3f, 0.3f, 0.3f };			//画面に収める(1で実寸)

	m_sprite = engine.CreateSprite(texture0, { 128.0f, 128.0f }, { 256.0f, 256.0f });

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
	Vector3 s = { m_sphere->transform.scale.x, m_sphere->transform.scale.y, m_sphere->transform.scale.z };
	Vector3 p = { m_sphere->transform.position.x, m_sphere->transform.position.y, m_sphere->transform.position.z };
	engine.DrawBox(p, s, { 1.0f, 1.0f, 0.0f, 1.0f });						//球を囲む箱(球は直径1×scale)
}

void GameApp::DrawUI(Engine& engine)
{
	EditModel("Triangle0", *m_triangle0);
	EditModel("Triangle1", *m_triangle1);
	EditModel("Sphere", *m_sphere);
	EditModel("Beetle", *m_beetle);

	ImGui::Begin("Sprite");
	ImGui::DragFloat2("Position", &m_sprite->position.x, 1.0f);	//ピクセル単位
	ImGui::DragFloat2("Size", &m_sprite->size.x, 1.0f);
	ImGui::DragFloat("Rotation", &m_sprite->rotation, 0.5f);	//度
	ImGui::ColorEdit4("Color", &m_sprite->color.x);
	ImGui::Checkbox("Visible", &m_sprite->visible);
	ImGui::End();

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
	if (ImGui::Checkbox("Follow Beetle", &follow))				//親子関係の確認
	{
		camera.transform.parent = follow ? &m_beetle->transform : nullptr;	//位置はビートルから見た値になる
	}
	ImGui::End();

	DebugCamera& debugCamera = engine.GetDebugCamera();
	ImGui::Begin("DebugCamera");								//値の確認・直接入力
	ImGui::Checkbox("Enabled (F1)", &debugCamera.enabled);
	ImGui::DragFloat3("Target", &debugCamera.target.x, 0.01f);
	ImGui::DragFloat("Yaw", &debugCamera.yaw, 0.5f);			//度
	ImGui::DragFloat("Pitch", &debugCamera.pitch, 0.5f, -89.0f, 89.0f);
	ImGui::DragFloat("Distance", &debugCamera.distance, 0.01f, 0.1f, 100.0f);
	ImGui::End();

	ImGui::Begin("Window");										//サイズ変更の確認
	ImGui::Text("Size %u x %u", engine.GetWindowWidth(), engine.GetWindowHeight());
	if (ImGui::Button("960x540"))   engine.SetWindowSize(960, 540);
	ImGui::SameLine();
	if (ImGui::Button("1280x720"))  engine.SetWindowSize(1280, 720);
	ImGui::SameLine();
	if (ImGui::Button("1920x1080")) engine.SetWindowSize(1920, 1080);
	ImGui::End();

	ImGui::ShowDemoWindow();									//動作確認用
}
