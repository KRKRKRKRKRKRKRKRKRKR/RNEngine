#include "Engine/DebugCamera.h"
#include "Renderer/Camera.h"
#include "OS/Input.h"
#include "imgui.h"
#include <windows.h>						//VK_RBUTTON
#include <algorithm>						//std::clamp
#include <cmath>							//sin, cos

void DebugCamera::Update(Camera& camera, const Input& input)
{
	const bool overUI = ImGui::GetIO().WantCaptureMouse;	//ImGuiの窓の上なら触らない

	//右ドラッグで回転(押した瞬間は移動量が当てにならないので飛ばす)
	if (!overUI && input.IsDown(VK_RBUTTON) && !input.IsPressed(VK_RBUTTON))
	{
		const float speed = 0.3f;						//1ピクセルで0.3度
		yaw += input.GetMouseDelta().x * speed;			//物をつかんで回す向き(右へ→物が右へ回る)
		pitch += input.GetMouseDelta().y * speed;		//下へ→物の上面が見える
		pitch = std::clamp(pitch, -89.0f, 89.0f);		//真上・真下はLookAtが壊れる
	}

	//ホイールで距離(倍率で変えると、近くでも遠くでも同じ感覚になる)
	if (!overUI && input.GetWheel() != 0.0f)
	{
		distance *= std::pow(0.9f, input.GetWheel());	//奥へ1目盛り→0.9倍(近づく)
		distance = std::clamp(distance, 0.1f, 100.0f);
	}

	//角度→向き(回転)。yaw,pitchがそのままY軸・X軸まわりの回転
	camera.transform.parent = nullptr;					//デバッグ中は何にも付いていかない
	camera.transform.rotation = { pitch, yaw, 0.0f };

	//角度と距離→位置(球面座標)。向いている方向の逆へdistance下がる
	const float toRad = 3.14159265f / 180.0f;
	const float y = yaw * toRad;
	const float p = pitch * toRad;
	camera.transform.position.x = target.x - distance * std::cos(p) * std::sin(y);
	camera.transform.position.y = target.y + distance * std::sin(p);
	camera.transform.position.z = target.z - distance * std::cos(p) * std::cos(y);	//yaw=0で-z側
}
