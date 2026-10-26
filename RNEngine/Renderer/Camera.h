#pragma once
#include <DirectXMath.h>
#include "Math/Vector.h"

//視点(位置と向き)と見え方(視野角など)を持つ
class Camera
{
public:
	Vector3 position = { 0.0f, 0.0f, -3.0f };	//カメラの位置(原点の手前)
	Vector3 target = { 0.0f, 0.0f, 0.0f };		//見ている点
	float fovY = 60.0f;							//縦の視野角(度)
	float nearZ = 0.1f;							//手前の限界
	float farZ = 100.0f;						//奥の限界

	//ビュー行列(世界→カメラから見た座標)
	DirectX::XMMATRIX GetViewMatrix() const
	{
		using namespace DirectX;
		XMVECTOR eye = XMVectorSet(position.x, position.y, position.z, 1.0f);
		XMVECTOR at = XMVectorSet(target.x, target.y, target.z, 1.0f);
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);	//上は+y
		return XMMatrixLookAtLH(eye, at, up);
	}

	//透視投影行列(カメラ座標→画面)。aspectは幅÷高さ
	DirectX::XMMATRIX GetProjectionMatrix(float aspect) const
	{
		using namespace DirectX;
		return XMMatrixPerspectiveFovLH(XMConvertToRadians(fovY), aspect, nearZ, farZ);
	}

	//ビュー×投影(まとめて掛けたもの)
	DirectX::XMMATRIX GetViewProjectionMatrix(float aspect) const
	{
		return GetViewMatrix() * GetProjectionMatrix(aspect);
	}
};
