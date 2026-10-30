#include "Game/Spinner.h"
#include "Engine/Model.h"
#include "Engine/Engine.h"

Spinner::Spinner(int texture, float x, float y)
	: m_texture(texture)
{
	transform.position = { x, y, 0.0f };				//ここではまだModelを作れない
	transform.scale = { 0.3f, 0.3f, 0.3f };
}

void Spinner::Init()
{
	m_model = CreateSphere(m_texture);					//Spawnの中で呼ばれる

	m_moon = CreateSphere(m_texture);					//親から見た位置を持つ
	m_moon->transform.position = { 2.0f, 0.0f, 0.0f };	//親の右(親の拡大0.3も掛かる)
	m_moon->transform.scale = { 0.3f, 0.3f, 0.3f };
}

void Spinner::Update(float dt)
{
	transform.rotation.y += 90.0f * dt;					//1秒で90度(Modelは親子関係で付いてくる)

	const Input& input = GetEngine().GetInput();
	const float speed = 1.0f * dt;						//1秒で1進む
	if (input.IsDown(VK_LEFT))  transform.position.x -= speed;	//押している間動く
	if (input.IsDown(VK_RIGHT)) transform.position.x += speed;
	if (input.IsDown(VK_UP))    transform.position.y += speed;
	if (input.IsDown(VK_DOWN))  transform.position.y -= speed;

	if (input.IsPressed(VK_SPACE))  Log::Info("Space pressed");	//1回だけ出る
	if (input.IsReleased(VK_SPACE)) Log::Info("Space released");
}
