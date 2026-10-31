#pragma once
#include <vector>							//std::vector
#include <memory>							//std::unique_ptr
#include "Common.h"
#include "OS/Window.h"
#include "OS/Input.h"
#include "Renderer/Renderer.h"
#include "Math/Transform.h"
#include "Math/Vector.h"
#include "Engine/Model.h"
#include "Engine/Sprite.h"
#include "Engine/GameObject.h"
#include "Engine/DebugCamera.h"

class Game;

//ウィンドウ・描画・資源を持ち、ゲームループを回す
class Engine
{
public:
	void Init(HINSTANCE hInstance, int nCmdShow, UINT width, UINT height, const wchar_t* title);
	void Run(Game& game);					//Init→ループ(Update→Draw→DrawUI)

	//資源(番号で扱う)
	int LoadTexture(const wchar_t* file);							//画像を読んで番号を返す
	int LoadMesh(const char* file, float scale = 1.0f);				//モデル(obj,fbx等)を読んで形の番号を返す(失敗は-1)。scale=単位合わせ(cm→mなら0.01)
	int GetTriangleMesh() const { return m_triangleMesh; }			//三角形の形の番号
	int GetQuadMesh() const { return m_quadMesh; }					//四角形の形の番号

	//物を作る(Engineが持ち、毎フレーム自動で描く。返すポインタは借りるだけ)
	Model* CreateModel(int mesh, int texture);						//LoadMeshの形
	Model* CreateTriangle(int texture);								//三角形
	Model* CreateSphere(int texture);								//球(半径0.5)
	Sprite* CreateSprite(int texture, const Vector2& position = { 640.0f, 360.0f }, const Vector2& size = { 100.0f, 100.0f });
	void Destroy(Model* model);										//一覧から削除(以後ポインタは使えない)
	void Destroy(Sprite* sprite);

	//GameObjectを出す(Tのコンストラクタに引数を渡す。動き出すのは次のフレームから)
	template<class T, class... Args>
	T* Spawn(Args&&... args)
	{
		std::unique_ptr<T> obj = std::make_unique<T>(std::forward<Args>(args)...);	//引数をそのまま渡す
		T* ptr = obj.get();
		obj->m_engine = this;
		obj->Init();									//ここでModelを作れる
		m_pendingObjects.push_back(std::move(obj));		//Update中の一覧を壊さないよう別に溜める
		return ptr;
	}

	//その場で描く(Game::Drawの中で呼ぶ。普段は不要)
	void DrawMesh(int mesh, int texture, const Transform& transform, const Vector4& color);	//3D
	void DrawSprite(const Vector2& position, const Vector2& size, float rotation, int texture, const Vector4& color);	//ピクセル座標(position=中心,rotation=度)

	Camera& GetCamera() { return m_camera; }						//ゲームのカメラ(動かす用)
	DebugCamera& GetDebugCamera() { return m_debugCamera; }			//F1で切り替え
	const Input& GetInput() const { return m_input; }				//キー入力(読むだけ)

private:
	int AddMesh();							//空の形を一覧に追加して番号を返す
	Model* AddModel(int mesh, int texture);	//Modelを一覧に追加
	void DrawModels();						//一覧の3Dを全部描く
	void DrawSprites();						//一覧の2Dを全部描く
	void UpdateObjects(float dt);			//追加→全部Update→削除

	int m_nCmdShow = SW_SHOWDEFAULT;		//ウィンドウの表示方法

	Window m_window;						//最初に宣言(最後に破棄)
	Input m_input;
	Renderer m_renderer;
	Pipeline m_pipeline;
	Camera m_camera;						//ゲームのカメラ
	DebugCamera m_debugCamera;				//マウスで見回す(enabledの間だけ使う)
	Camera m_debugView;						//デバッグカメラの結果(ゲームのカメラを壊さない)
	std::vector<Mesh> m_meshes;				//形の一覧
	std::vector<Texture> m_textures;		//画像の一覧
	int m_triangleMesh = -1;				//組み込みの三角形
	int m_quadMesh = -1;					//組み込みの四角形(スプライト用)
	int m_sphereMesh = -1;					//組み込みの球(全ての球で共有)

	std::vector<std::unique_ptr<Model>> m_models;		//3Dの物(Engineが持ち主)
	std::vector<std::unique_ptr<Sprite>> m_sprites;		//2Dの物

	//m_modelsより後に宣言(先に破棄され、そのときModelを消せる)
	std::vector<std::unique_ptr<GameObject>> m_objects;			//動いている物
	std::vector<std::unique_ptr<GameObject>> m_pendingObjects;	//出現待ち
};
