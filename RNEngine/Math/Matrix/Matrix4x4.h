#pragma once
#include <cmath>
#include "Math/Vector.h"

//4x4行列
struct Matrix4x4
{
	//m[行][列]
	float m[4][4] = {};  

	//単位行列
	static Matrix4x4 Identity()
	{
		Matrix4x4 r;
		r.m[0][0] = 1.0f;
		r.m[1][1] = 1.0f;
		r.m[2][2] = 1.0f;
		r.m[3][3] = 1.0f;
		return r;
	}

	//行列の積
	Matrix4x4 operator*(const Matrix4x4& b) const
	{
		Matrix4x4 r;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				for (int k = 0; k < 4; k++)
				{
					//(行i)と(列j)の内積
					r.m[i][j] += m[i][k] * b.m[k][j];  
				}
			}
		}
		return r;
	}

	//加算
	Matrix4x4 operator+(const Matrix4x4& b) const
	{
		Matrix4x4 r;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				r.m[i][j] = m[i][j] + b.m[i][j];
			}
		}
		return r;
	}

	//減算
	Matrix4x4 operator-(const Matrix4x4& b) const
	{
		Matrix4x4 r;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				r.m[i][j] = m[i][j] - b.m[i][j];
			}
		}
		return r;
	}

	//スカラー倍
	Matrix4x4 operator*(float s) const
	{
		Matrix4x4 r;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				r.m[i][j] = m[i][j] * s;
			}
		}
		return r;
	}

	//転置
	Matrix4x4 Transpose() const
	{
		Matrix4x4 r;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				r.m[i][j] = m[j][i];
			}
		}
		return r;
	}

	//Vector4に適用
	Vector4 Transform(const Vector4& v) const
	{
		return {
			v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0] + v.w * m[3][0],
			v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1] + v.w * m[3][1],
			v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2] + v.w * m[3][2],
			v.x * m[0][3] + v.y * m[1][3] + v.z * m[2][3] + v.w * m[3][3]
		};
	}

	//位置に適用
	Vector3 TransformPoint(const Vector3& p) const
	{
		Vector4 r = Transform({ p.x, p.y, p.z, 1.0f });
		return { r.x, r.y, r.z };
	}

	//方向に適用
	Vector3 TransformDirection(const Vector3& d) const
	{
		Vector4 r = Transform({ d.x, d.y, d.z, 0.0f });
		return { r.x, r.y, r.z };
	}

	//移動
	static Matrix4x4 Translation(const Vector3& t)
	{
		Matrix4x4 r = Identity();
		r.m[3][0] = t.x;  //最下行に移動量
		r.m[3][1] = t.y;
		r.m[3][2] = t.z;
		return r;
	}

	//拡大
	static Matrix4x4 Scaling(const Vector3& s)
	{
		Matrix4x4 r = Identity();
		r.m[0][0] = s.x;  //対角に拡大率
		r.m[1][1] = s.y;
		r.m[2][2] = s.z;
		return r;
	}

	//X軸まわりの回転(ラジアン)
	static Matrix4x4 RotationX(float rad)
	{
		Matrix4x4 r = Identity();
		float c = std::cos(rad);
		float s = std::sin(rad);
		r.m[1][1] = c;  r.m[1][2] = s;
		r.m[2][1] = -s; r.m[2][2] = c;
		return r;
	}

	//Y軸まわりの回転(ラジアン)
	static Matrix4x4 RotationY(float rad)
	{
		Matrix4x4 r = Identity();
		float c = std::cos(rad);
		float s = std::sin(rad);
		r.m[0][0] = c; r.m[0][2] = -s;
		r.m[2][0] = s; r.m[2][2] = c;
		return r;
	}

	//Z軸まわりの回転(ラジアン)
	static Matrix4x4 RotationZ(float rad)
	{
		Matrix4x4 r = Identity();
		float c = std::cos(rad);
		float s = std::sin(rad);
		r.m[0][0] = c;  r.m[0][1] = s;
		r.m[1][0] = -s; r.m[1][1] = c;
		return r;
	}
};
