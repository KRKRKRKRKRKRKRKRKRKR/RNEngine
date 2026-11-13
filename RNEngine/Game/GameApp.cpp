#include "Game/GameApp.h"
#include "Engine/Engine.h"
#include "imgui.h"
#include <algorithm>						//std::max


void GameApp::Init(Engine& engine)
{
	texture0 = engine.LoadTexture(L"Resource/UVChecker0.png");	//画像(番号を受け取る)
	texture1 = engine.LoadTexture(L"Resource/UVChecker1.png");
	textureWhite = engine.LoadTexture(L"Resource/white1x1.png");

	for(int i = 0; i < 10; i++)
	{
		m_sprite = engine.CreateSprite(texture0, { 128.0f+ i*30, 128.0f + i*30 }, { 256.0f, 256.0f });
	}
	Camera& camera = engine.GetCamera();
	camera.transform.position = { 2.0f,2.0f,-2.0f };
	camera.transform.rotation = { 35.0f,-45.0f,0.0f };
	camera.fovY = 60;

	const Vector4 colors[BALL_COUNT] =
	{
		{ 0.8f, 0.0f, 0.0f, 0.2f },								//Red
		{ 0.0f, 0.8f, 0.0f, 0.2f },								//Green
		{ 0.0f, 0.0f, 0.8f, 0.2f },								//Blue
	};
	for (int i = 0; i < BALL_COUNT; i++)
	{
		Ball ball;
		ball.sprite = engine.CreateSprite(textureWhite, { 0.0f, 0.0f }, { 48.0f, 48.0f });
		ball.sprite->color = colors[i];							//白画像に色を掛ける
		ResetBall(engine, ball);
		m_balls.push_back(std::move(ball));
	}
}

float GameApp::RandomRange(float min, float max)
{
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(m_random);
}

//左端から、毎回違う動きで落とし始める
void GameApp::ResetBall(Engine& engine, Ball& ball)
{
	const float radius = ball.sprite->size.y * 0.5f;
	const float height = (float)engine.GetWindowHeight();

	ball.sprite->position = { radius, RandomRange(radius, (std::max)(radius, height * 0.4f)) };	//左端・高さはランダム(上のほう)。(std::max)はwindows.hのmaxマクロ避け
	ball.speedX = RandomRange(150.0f, 500.0f);				//右へ進む速さ
	ball.speedY = 0.0f;										//落とし始めは縦の速さ0
	ball.gravity = RandomRange(1000.0f, 2000.0f);			//重力の強さ
	ball.bounce = RandomRange(0.7f, 0.95f);					//跳ね返りの強さ
	ball.trailTimer = 0.0f;
}

void GameApp::UpdateBall(Engine& engine, Ball& ball, float dt)
{
	//ボールの動き
	ball.speedY += ball.gravity * dt;						//重力で下向きに加速
	ball.sprite->position.x += ball.speedX * dt;			//速さ×時間だけ進む
	ball.sprite->position.y += ball.speedY * dt;

	const float radius = ball.sprite->size.y * 0.5f;
	const float floorY = (float)engine.GetWindowHeight();	//床=窓の下端
	if (ball.sprite->position.y + radius > floorY)			//床にめり込んだ
	{
		ball.sprite->position.y = floorY - radius;			//床の上に戻す
		ball.speedY = -ball.speedY * ball.bounce;			//向きを反転して少し弱める
	}
	if (ball.sprite->position.x - radius > (float)engine.GetWindowWidth())	//右へ出きった
	{
		ResetBall(engine, ball);							//また左から、新しい動きで
	}

	//軌跡の寿命
	for (TrailPoint& point : ball.trail)
	{
		point.age += dt;									//残してからの時間を進める
	}
	std::erase_if(ball.trail, [this](const TrailPoint& point) { return point.age >= m_trailLifeTime; });	//寿命が来たら消す

	//軌跡を一定間隔で残す
	ball.trailTimer += dt;
	if (ball.trailTimer >= m_trailInterval)
	{
		ball.trailTimer -= m_trailInterval;
		ball.trail.push_back({ ball.sprite->position, 0.0f });	//生まれた瞬間は経過0
	}
}

void GameApp::Update(Engine& engine, float dt)
{
	//ゲーム全体のルール(GameObjectのUpdateの後に呼ばれる)

	if (dt > 0.05f) dt = 0.05f;								//窓を掴んだ後などの長いdtで床を抜けないように

	for (Ball& ball : m_balls)
	{
		UpdateBall(engine, ball, dt);
	}
}

//ボールが通った場所に、ボールと同じ大きさ・色の板ポリを並べる(後に描いたものが手前)
void GameApp::DrawTrail(Engine& engine)
{
	for (const Ball& ball : m_balls)
	{
		for (const TrailPoint& point : ball.trail)
		{
			engine.DrawSprite(point.position, ball.sprite->size, 0.0f, textureWhite, ball.sprite->color);
		}
	}
}

void GameApp::Draw(Engine& engine)
{
	DrawTrail(engine);										//ボール(自動描画)より先に描く=ボールが手前

	//その場で描く分(自動描画のModelは書かなくてよい)
	engine.DrawLine({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });	//x軸 赤
	engine.DrawLine({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f });	//y軸 緑
	engine.DrawLine({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f });	//z軸 青

	engine.DrawGrid();														//床(10x10、1マス1)
}

void GameApp::DrawUI(Engine& engine)
{

	const Input& input = engine.GetInput();
	ImGui::Begin("Ball");										//ソリティア風の演出
	ImGui::DragFloat("TrailInterval", &m_trailInterval, 0.001f, 0.005f, 0.2f);
	ImGui::DragFloat("TrailLifeTime", &m_trailLifeTime, 0.05f, 0.1f, 10.0f);	//秒
	for (size_t i = 0; i < m_balls.size(); i++)					//今の動き(落とし直すたびに変わる)
	{
		const Ball& ball = m_balls[i];
		ImGui::Text("Ball%d SpeedX %.0f Gravity %.0f Bounce %.2f Trail %d",
			(int)i, ball.speedX, ball.gravity, ball.bounce, (int)ball.trail.size());
	}
	if (ImGui::Button("Respawn"))								//全員を落とし直す
	{
		for (Ball& ball : m_balls)
		{
			ResetBall(engine, ball);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear Trail"))
	{
		for (Ball& ball : m_balls)
		{
			ball.trail.clear();
		}
	}
	ImGui::End();

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
