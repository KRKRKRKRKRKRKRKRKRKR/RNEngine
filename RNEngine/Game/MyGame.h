#pragma once
#include <vector>							//std::vector
#include "Engine/Game.h"
#include "Engine/GameObject.h"
#include "Math/Transform.h"
#include "Math/Vector.h"

//実際のゲームの中身
class MyGame : public Game
{
public:
	void Init(Engine& engine) override;
	void Draw(Engine& engine) override;
	void DrawUI(Engine& engine) override;

private:
	void AddTriangle(Engine& engine, const Transform& transform, const Vector4& color);	//三角形を一覧に追加

	int m_texture = -1;						//画像の番号
	std::vector<GameObject> m_objects;		//三角形たち

	Vector2 m_spritePosition = { 640.0f, 360.0f };			//中心のピクセル位置
	Vector2 m_spriteSize = { 256.0f, 256.0f };				//ピクセルサイズ
	float m_spriteRotation = 0.0f;							//回転(度)
	Vector4 m_spriteColor = { 1.0f, 1.0f, 1.0f, 1.0f };		//RGBA

	int m_sphereMesh = -1;									//球体の形の番号
	Transform m_sphereTransform;							//球体のSRT
	Vector4 m_sphereColor = { 1.0f, 1.0f, 1.0f, 1.0f };		//RGBA
};
