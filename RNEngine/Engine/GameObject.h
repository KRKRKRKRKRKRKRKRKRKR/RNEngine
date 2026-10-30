#pragma once
#include "Math/Transform.h"
#include "Math/Vector.h"

//形・画像・SRT・色をまとめた描画できる物
struct GameObject
{
	int mesh = -1;								//形の番号(Engineが持つ一覧の添字)
	int texture = -1;							//画像の番号(Engineが持つ一覧の添字)
	Transform transform;						//位置・回転・拡大
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };	//RGBA(画像に掛ける色)

	GameObject(int mesh, int texture, const Transform& transform = Transform(), const Vector4& color = { 1.0f, 1.0f, 1.0f, 1.0f })
		: mesh(mesh), texture(texture), transform(transform), color(color) {}	//引数をそのままメンバへ
};
