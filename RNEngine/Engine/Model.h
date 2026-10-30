#pragma once
#include "Math/Transform.h"
#include "Math/Vector.h"

//3Dの物(Engineが持ち、自動で描く)
class Model
{
public:
	Transform transform;								//位置・回転・拡大
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };			//RGBA(画像に掛ける色)
	int texture = -1;									//画像の番号
	bool visible = true;								//falseなら描かない

private:
	friend class Engine;								//m_meshはEngineだけが設定する
	int m_mesh = -1;									//形の番号(作った後は変えない)
};
