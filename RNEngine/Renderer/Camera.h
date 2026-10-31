#pragma once
#include <DirectXMath.h>
#include "Math/Transform.h"

//視点(位置と向き)と見え方(視野角など)を持つ
class Camera
{
public:
	Transform transform = { .position = { 0.0f, 0.0f, -3.0f } };	//位置・回転(度)。+zが前。parentで付いていく
	float fovY = 60.0f;							//縦の視野角(度)
	float nearZ = 0.1f;							//手前の限界
	float farZ = 100.0f;						//奥の限界

	//ビュー行列(世界→カメラから見た座標)=カメラのワールド行列の逆
	DirectX::XMMATRIX GetViewMatrix() const
	{
		using namespace DirectX;
		XMVECTOR s, r, t;
		XMMatrixDecompose(&s, &r, &t, transform.GetWorldMatrix());	//拡大・回転・移動に分ける
		XMMATRIX world = XMMatrixRotationQuaternion(r) * XMMatrixTranslationFromVector(t);	//拡大は捨てる(親の拡大で画面が歪まない)
		return XMMatrixInverse(nullptr, world);	//カメラを原点に戻す動き
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
