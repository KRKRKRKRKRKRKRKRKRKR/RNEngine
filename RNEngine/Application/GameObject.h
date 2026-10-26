#pragma once
#pragma once
#include "Renderer/Mesh.h"
#include "Math/Transform.h"

//ゲームオブジェクトの形とSRTを持つ
struct GameObject
{
	Mesh mesh;				//形
	Transform transform;	//位置・回転・拡大
};
