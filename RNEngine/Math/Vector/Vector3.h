#pragma once
#include <cmath>

//3次元ベクトル
struct Vector3
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	//四則演算	

	//加算
	Vector3 operator+(const Vector3& v) const
	{
		return { x + v.x, y + v.y, z + v.z };
	}

	//減算
	Vector3 operator-(const Vector3& v) const
	{
		return { x - v.x, y - v.y, z - v.z };
	}

	//成分ごとの掛け算
	Vector3 operator*(const Vector3& v) const 
	{ 
		return { x * v.x, y * v.y, z * v.z }; 
	}  
	
	//成分ごとの割り算
	Vector3 operator/(const Vector3& v) const 
	{
		return { x / v.x, y / v.y, z / v.z }; 
	}

	//スカラー倍
	Vector3 operator*(float s) const
	{ 
		return { x * s, y * s, z * s }; 
	} 
	
	//スカラー除算
	Vector3 operator/(float s) const 
	{ 
		return { x / s, y / s, z / s }; 
	}             
	
	//符号を反転
	Vector3 operator-() const 
	{ 
		return { -x, -y, -z }; 
	}                            

	//ベクトル演算
	//内積
	float Dot(const Vector3& v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	//外積
	Vector3 Cross(const Vector3& v) const
	{
		return {
			y * v.z - z * v.y,
			z * v.x - x * v.z,
			x * v.y - y * v.x
		};
	}

	//長さ
	float Length() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	//正規化
	Vector3 Normalize() const
	{
		float len = Length();
		if (len == 0.0f) return { 0.0f, 0.0f, 0.0f };
		return { x / len, y / len, z / len };
	}

};

