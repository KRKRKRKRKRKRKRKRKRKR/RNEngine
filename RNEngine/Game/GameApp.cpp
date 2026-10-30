#include "Game/GameApp.h"
#include "Engine/Engine.h"
#include "imgui.h"

void GameApp::Init(Engine& engine)
{
	m_texture = engine.LoadTexture(L"Resource/UVChecker0.png");	//画像(番号を受け取る)

	Transform t1;
	AddTriangle(engine, t1, Vector4{ 1.0f, 1.0f, 1.0f, 1.0f });	//白=画像そのまま

	Transform t2;
	t2.position.x = 0.5f;
	AddTriangle(engine, t2, Vector4{ 1.0f, 0.5f, 0.5f, 1.0f });	//同じ画像を赤みで

	m_sphereMesh = engine.CreateSphere(0.5f, 32, 16);			//球体の形(半径0.5)
	m_sphereTransform.scale = { 0.5f, 0.5f, 0.5f };				//小さくして画面に収める
}

void GameApp::AddTriangle(Engine& engine, const Transform& transform, const Vector4& color)
{
	m_objects.emplace_back(engine.GetTriangleMesh(), m_texture, transform, color);
}

void GameApp::Draw(Engine& engine)
{
	for (const GameObject& obj : m_objects)
	{
		engine.DrawMesh(obj.mesh, obj.texture, obj.transform, obj.color);	//全部描く
	}
	engine.DrawSprite(m_spritePosition, m_spriteSize, m_spriteRotation, m_texture, m_spriteColor);	//スプライト
	engine.DrawMesh(m_sphereMesh, m_texture, m_sphereTransform, m_sphereColor);	//球体
}

void GameApp::DrawUI(Engine& engine)
{
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
}
