#pragma once
#include <cmath>
#include "Math/Vector/Vector3.h"

//3x3行列
struct Matrix3x3
{
	//m[行][列]
	float m[3][3] = {}; 

	//単位行列(掛けても何も変わない)
	static Matrix3x3 Identity()
	{
		Matrix3x3 r;
		r.m[0][0] = 1.0f;
		r.m[1][1] = 1.0f;
		r.m[2][2] = 1.0f;
		return r;
	}

	//行列の積
	Matrix3x3 operator*(const Matrix3x3& b) const
	{
		Matrix3x3 r;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					//(行i)と(列j)の内積
					r.m[i][j] += m[i][k] * b.m[k][j];  
				}
			}
		}
		return r;
	}

	//加算
	Matrix3x3 operator+(const Matrix3x3& b) const
	{
		Matrix3x3 r;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				r.m[i][j] = m[i][j] + b.m[i][j];
			}
		}
		return r;
	}

	//減算
	Matrix3x3 operator-(const Matrix3x3& b) const
	{
		Matrix3x3 r;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				r.m[i][j] = m[i][j] - b.m[i][j];
			}
		}
		return r;
	}

	//スカラー倍
	Matrix3x3 operator*(float s) const
	{
		Matrix3x3 r;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				r.m[i][j] = m[i][j] * s;
			}
		}
		return r;
	}

	//転置
	Matrix3x3 Transpose() const
	{
		Matrix3x3 r;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				r.m[i][j] = m[j][i];
			}
		}
		return r;
	}

	//行列式
	float Determinant() const
	{
		return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
			- m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
			+ m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
	}

	//ベクトルに適用
	Vector3 Transform(const Vector3& v) const
	{
		return {
			v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0],
			v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1],
			v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2]
		};
	}

	//拡大(3x3は移動を持てない)
	static Matrix3x3 Scaling(const Vector3& s)
	{
		Matrix3x3 r = Identity();
		r.m[0][0] = s.x;
		r.m[1][1] = s.y;
		r.m[2][2] = s.z;
		return r;
	}

	//X軸まわりの回転(ラジアン)
	static Matrix3x3 RotationX(float rad)
	{
		Matrix3x3 r = Identity();
		float c = std::cos(rad);
		float s = std::sin(rad);
		r.m[1][1] = c;  r.m[1][2] = s;
		r.m[2][1] = -s; r.m[2][2] = c;
		return r;
	}

	//Y軸まわりの回転(ラジアン)
	static Matrix3x3 RotationY(float rad)
	{
		Matrix3x3 r = Identity();
		float c = std::cos(rad);
		float s = std::sin(rad);
		r.m[0][0] = c; r.m[0][2] = -s;
		r.m[2][0] = s; r.m[2][2] = c;
		return r;
	}

	//Z軸まわりの回転(ラジアン)
	static Matrix3x3 RotationZ(float rad)
	{
		Matrix3x3 r = Identity();
		float c = std::cos(rad);
		float s = std::sin(rad);
		r.m[0][0] = c;  r.m[0][1] = s;
		r.m[1][0] = -s; r.m[1][1] = c;
		return r;
	}
};
