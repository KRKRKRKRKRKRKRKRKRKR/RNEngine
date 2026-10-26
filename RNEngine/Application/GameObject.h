#pragma once
#include "Renderer/Mesh.h"
#include "Math/Transform.h"
#include "Math/Vector.h"
//ゲームオブジェクトの形とSRTを持つ
struct GameObject
{
	const Mesh* mesh = nullptr;					//形(参照のみ。持ち主はApplication)
	int texture = -1;							//画像の番号(Applicationが持つ一覧の添字)
	Transform transform;						//位置・回転・拡大
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };	//RGBA(画像に掛ける色)

	//コンストラクタ

	GameObject(const Mesh* mesh, int texture,const Transform& transform = Transform(), const Vector4& color = { 1.0f, 1.0f, 1.0f, 1.0f }): mesh(mesh), texture(texture), transform(transform), color(color) {
	}	//引数をそのままメンバへ
};
