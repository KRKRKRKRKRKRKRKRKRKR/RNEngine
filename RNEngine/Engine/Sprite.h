#pragma once
#include "Math/Vector.h"

//2Dの画像(Engineが持ち、自動で描く)
struct Sprite
{
	Vector2 position = { 0.0f, 0.0f };					//中心(ピクセル)
	Vector2 size = { 100.0f, 100.0f };					//ピクセル
	float rotation = 0.0f;								//度
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };			//RGBA
	int texture = -1;									//画像の番号
	bool visible = true;								//falseなら描かない
};
