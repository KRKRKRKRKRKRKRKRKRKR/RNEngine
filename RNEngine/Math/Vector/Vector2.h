#pragma once
#include <cmath>

//2次元ベクトル
struct Vector2
{
	float x = 0.0f;
	float y = 0.0f;

	//四則演算

	//加算
	Vector2 operator+(const Vector2& v) const
	{
		return { x + v.x, y + v.y };
	}

	//減算
	Vector2 operator-(const Vector2& v) const
	{
		return { x - v.x, y - v.y };
	}

	//成分ごとの掛け算
	Vector2 operator*(const Vector2& v) const
	{
		return { x * v.x, y * v.y };
	}

	//成分ごとの割り算
	Vector2 operator/(const Vector2& v) const
	{
		return { x / v.x, y / v.y };
	}

	//スカラー倍
	Vector2 operator*(float s) const
	{
		return { x * s, y * s };
	}

	//スカラー除算
	Vector2 operator/(float s) const
	{
		return { x / s, y / s };
	}

	//符号を反転
	Vector2 operator-() const
	{
		return { -x, -y };
	}

	//ベクトル演算
	//内積
	float Dot(const Vector2& v) const
	{
		return x * v.x + y * v.y;
	}

	//外積(2Dではスカラー)
	float Cross(const Vector2& v) const
	{
		return x * v.y - y * v.x;
	}

	//長さ
	float Length() const
	{
		return std::sqrt(x * x + y * y);
	}

	//正規化
	Vector2 Normalize() const
	{
		float len = Length();
		if (len == 0.0f) return { 0.0f, 0.0f };
		return { x / len, y / len };
	}

};
