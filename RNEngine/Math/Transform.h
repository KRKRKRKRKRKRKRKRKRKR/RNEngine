#pragma once
#include <DirectXMath.h>					//行列・ベクトル計算

//位置・回転・拡大を持ち、ワールド行列を作る
struct Transform
{
	DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };	//移動
	DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };	//回転(度) xyz
	DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };		//拡大

	//ローカル→ワールドの行列(S→R→Tの順)
	DirectX::XMMATRIX GetWorldMatrix() const
	{
		using namespace DirectX;
		XMMATRIX s = XMMatrixScaling(scale.x, scale.y, scale.z);
		XMMATRIX r = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(rotation.x),		//X軸まわり(pitch)
			XMConvertToRadians(rotation.y),		//Y軸まわり(yaw)
			XMConvertToRadians(rotation.z));	//Z軸まわり(roll)
		XMMATRIX t = XMMatrixTranslation(position.x, position.y, position.z);
		return s * r * t;
	}
};