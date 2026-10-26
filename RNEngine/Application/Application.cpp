#include "Application/Application.h"
#include "imgui.h"

void Application::Init(HINSTANCE hInstance, int nCmdShow)
{
	m_window.Init(hInstance, WIDTH, HEIGHT, L"RNEngine");
	m_renderer.Init(m_window.GetHwnd(), WIDTH, HEIGHT);	//GPU・画面バッファ

	m_pipeline.Init(m_renderer.GetDevice(), L"shaders.hlsl", m_renderer.GetFormat());	//描き方

	m_triangleMesh.InitTriangle(m_renderer.GetDevice());	//三角形の形(1回だけ)
	m_quadMesh.InitQuad(m_renderer.GetDevice());
	m_texture = LoadTexture(L"Resource/UVChecker.png");		//画像(番号を受け取る)

	Transform t1;
	Triangle(t1, m_texture, Vector4{ 1.0f, 1.0f, 1.0f, 1.0f });	//白=画像そのまま

	Transform t2;
	t2.position.x = 0.5f;
	Triangle(t2, m_texture, Vector4{ 1.0f, 0.5f, 0.5f, 1.0f });	//同じ画像を赤みで


	m_sphereMesh.InitSphere(m_renderer.GetDevice(), 0.5f, 32, 16);	//球体の形(半径0.5)

	m_sphereTransform.position.z = 0.5f;		//手前で切れないように奥へ(下の説明参照)
	m_sphereTransform.scale = { 0.5f, 0.5f, 0.5f };	//小さくして画面に収める


	m_window.Show(nCmdShow);
}

void Application::Run()
{
	while (m_window.ProcessMessages())		//falseで終了
	{
		Draw();
	}
}

//画像を読み込んで、番号を返す
int Application::LoadTexture(const wchar_t* file)
{
	m_textures.emplace_back();										//末尾に1枚分追加
	m_textures.back().Init(m_renderer.GetGraphicsDevice(), file);	//読み込み+GPUへ転送
	return (int)m_textures.size() - 1;								//追加した場所=番号
}

//三角形を一覧に追加
void Application::Triangle(const Transform& transform, int texture, const Vector4& color)
{
	m_objects.emplace_back(&m_triangleMesh, texture, transform, color);
}

//スプライトをその場で描く(positionは中心のピクセル位置、sizeはピクセルサイズ、rotationは度)
void Application::Sprite(const Vector2& position, const Vector2& size, float rotation, int texture, const Vector4& color)
{
	Transform t;
	t.position.x = position.x;				//中心のピクセル位置
	t.position.y = position.y;
	t.scale.x = size.x;						//1x1の板を、ピクセルサイズに拡大
	t.scale.y = size.y;
	t.rotation.z = rotation;				//画面の回転(Z軸まわり)
	m_renderer.DrawSprite(m_pipeline, m_quadMesh, m_textures[texture], t, color);
}

void Application::Draw()
{
	m_renderer.BeginFrame();

	for (const GameObject& obj : m_objects)
	{
		m_renderer.Draw(m_pipeline, *obj.mesh, m_textures[obj.texture], obj.transform, obj.color);	//全部描く

	}
	Sprite(m_spritePosition, m_spriteSize, m_spriteRotation, m_texture, m_spriteColor);	//スプライト
	m_renderer.Draw(m_pipeline, m_sphereMesh, m_textures[m_texture], m_sphereTransform, m_sphereColor);	//球体

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
		ImGui::ColorEdit4("Color", &obj.color.x);				//RGBA
		if (ImGui::Button("Reset"))
		{
			obj.transform = Transform();						//初期値に戻す
		}
		ImGui::PopID();
	}
	ImGui::End();

	ImGui::Begin("Sprite");
	ImGui::DragFloat2("Position", &m_spritePosition.x, 1.0f);	//ピクセル単位
	ImGui::DragFloat2("Size", &m_spriteSize.x, 1.0f);
	ImGui::DragFloat("Rotation", &m_spriteRotation, 0.5f);		//度
	ImGui::ColorEdit4("Color", &m_spriteColor.x);
	ImGui::End();


	ImGui::Begin("Sphere");
	ImGui::DragFloat3("Position", &m_sphereTransform.position.x, 0.01f);
	ImGui::DragFloat3("Rotation", &m_sphereTransform.rotation.x, 0.5f);
	ImGui::DragFloat3("Scale", &m_sphereTransform.scale.x, 0.01f);
	ImGui::ColorEdit4("Color", &m_sphereColor.x);
	ImGui::End();

	ImGui::ShowDemoWindow();									//動作確認用

	m_renderer.EndUI();

	m_renderer.EndFrame();
}
