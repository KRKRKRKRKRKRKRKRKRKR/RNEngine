#pragma once
#include "Math/Vector.h"

class Camera;
class Input;

//マウスで注視点の周りを回るカメラ(右ドラッグ=回転、ホイール=距離)
class DebugCamera
{
public:
	void Update(Camera& camera, const Input& input);	//入力で角度・距離を変え、cameraに書き込む

	bool enabled = false;						//trueの間、ゲームのカメラの代わりに使う(F1)
	Vector3 target = { 0.0f, 0.0f, 0.0f };		//回る中心
	float yaw = 0.0f;							//横の角度(度)。0で-z側から見る
	float pitch = 0.0f;							//縦の角度(度)。+で上から見下ろす
	float distance = 3.0f;						//中心からの距離
};
