#pragma once
#include <cmath>

//4次元ベクトル
struct Vector4
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;

	//四則演算

	//加算
	Vector4 operator+(const Vector4& v) const
	{
		return { x + v.x, y + v.y, z + v.z, w + v.w };
	}

	//減算
	Vector4 operator-(const Vector4& v) const
	{
		return { x - v.x, y - v.y, z - v.z, w - v.w };
	}

	//成分ごとの掛け算
	Vector4 operator*(const Vector4& v) const
	{
		return { x * v.x, y * v.y, z * v.z, w * v.w };
	}

	//成分ごとの割り算
	Vector4 operator/(const Vector4& v) const
	{
		return { x / v.x, y / v.y, z / v.z, w / v.w };
	}

	//スカラー倍
	Vector4 operator*(float s) const
	{
		return { x * s, y * s, z * s, w * s };
	}

	//スカラー除算
	Vector4 operator/(float s) const
	{
		return { x / s, y / s, z / s, w / s };
	}

	//符号を反転
	Vector4 operator-() const
	{
		return { -x, -y, -z, -w };
	}

	//ベクトル演算
	//内積
	float Dot(const Vector4& v) const
	{
		return x * v.x + y * v.y + z * v.z + w * v.w;
	}

	//長さ
	float Length() const
	{
		return std::sqrt(x * x + y * y + z * z + w * w);
	}

	//正規化
	Vector4 Normalize() const
	{
		float len = Length();
		if (len == 0.0f) return { 0.0f, 0.0f, 0.0f, 0.0f };
		return { x / len, y / len, z / len, w / len };
	}

};
