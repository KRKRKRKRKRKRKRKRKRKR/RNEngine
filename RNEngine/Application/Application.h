#pragma once
#include <vector> //std::vector
#include "Common.h"
#include "OS/Window.h"
#include "Renderer/Renderer.h"
#include  "Math/Transform.h"
#include "GameObject.h"
#include "Renderer/Texture.h"
#include "Math/Vector.h"

//アプリの流れ(初期化→ループ→終了)を回す
class Application
{
public:
	void Init(HINSTANCE hInstance, int nCmdShow);
	void Run();								//メインループ

private:
	void Triangle(const Transform& transform, int texture, const Vector4& color);	//三角形を追加
	int LoadTexture(const wchar_t* file);	//画像を読んで番号を返す
	void Draw();							//1フレームの描画
	void Sprite(const Vector2& position, const Vector2& size, float rotation, int texture, const Vector4& color);	//スプライトを描く

	static constexpr UINT WIDTH = 1280;		//描画幅
	static constexpr UINT HEIGHT = 720;		//描画高さ

	Window m_window;						//最初に宣言(最後に破棄)
	Renderer m_renderer;
	Pipeline m_pipeline;


	Mesh m_triangleMesh;						//三角形の形
	Mesh m_quadMesh;							//四角形の形
	std::vector<Texture> m_textures;			//読み込んだ画像の一覧
	std::vector<GameObject> m_objects;			//描画する物の一覧
	int m_texture = -1;

	Vector2 m_spritePosition = { 640.0f, 360.0f };			//中心のピクセル位置
	Vector2 m_spriteSize = { 256.0f, 256.0f };				//ピクセルサイズ
	float m_spriteRotation = 0.0f;							//回転(度)
	Vector4 m_spriteColor = { 1.0f, 1.0f, 1.0f, 1.0f };		//RGBA

};