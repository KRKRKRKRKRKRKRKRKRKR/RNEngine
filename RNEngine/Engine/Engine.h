#pragma once
#include <vector>							//std::vector
#include "Common.h"
#include "OS/Window.h"
#include "Renderer/Renderer.h"
#include "Math/Transform.h"
#include "Math/Vector.h"

class Game;

//ウィンドウ・描画・資源を持ち、ゲームループを回す
class Engine
{
public:
	void Init(HINSTANCE hInstance, int nCmdShow, UINT width, UINT height, const wchar_t* title);
	void Run(Game& game);					//Init→ループ(Update→Draw→DrawUI)

	//資源(番号で扱う)
	int LoadTexture(const wchar_t* file);							//画像を読んで番号を返す
	int CreateSphere(float radius, UINT slices, UINT stacks);		//球の形を作って番号を返す
	int GetTriangleMesh() const { return m_triangleMesh; }			//三角形の形の番号
	int GetQuadMesh() const { return m_quadMesh; }					//四角形の形の番号

	//描画(Game::Drawの中で呼ぶ)
	void DrawMesh(int mesh, int texture, const Transform& transform, const Vector4& color);	//3D
	void DrawSprite(const Vector2& position, const Vector2& size, float rotation, int texture, const Vector4& color);	//ピクセル座標(position=中心,rotation=度)

	Camera& GetCamera() { return m_renderer.GetCamera(); }			//カメラを動かす用

private:
	int AddMesh();							//空の形を一覧に追加して番号を返す

	int m_nCmdShow = SW_SHOWDEFAULT;		//ウィンドウの表示方法

	Window m_window;						//最初に宣言(最後に破棄)
	Renderer m_renderer;
	Pipeline m_pipeline;
	std::vector<Mesh> m_meshes;				//形の一覧
	std::vector<Texture> m_textures;		//画像の一覧
	int m_triangleMesh = -1;				//組み込みの三角形
	int m_quadMesh = -1;					//組み込みの四角形(スプライト用)
};
