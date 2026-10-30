#include "Engine/Engine.h"
#include "Engine/Game.h"
#include "Renderer/ModelLoader.h"
#include <chrono>							//経過時間

void Engine::Init(HINSTANCE hInstance, int nCmdShow, UINT width, UINT height, const wchar_t* title)
{
	m_nCmdShow = nCmdShow;

	m_window.Init(hInstance, width, height, title);
	m_renderer.Init(m_window.GetHwnd(), width, height);	//GPU・画面バッファ

	m_pipeline.Init(m_renderer.GetDevice(), L"shaders.hlsl", m_renderer.GetFormat(), m_renderer.GetDepthFormat());	//描き方

	m_triangleMesh = AddMesh();
	m_meshes[m_triangleMesh].InitTriangle(m_renderer.GetDevice());	//三角形の形
	m_quadMesh = AddMesh();
	m_meshes[m_quadMesh].InitQuad(m_renderer.GetDevice());			//四角形の形
	m_sphereMesh = AddMesh();
	m_meshes[m_sphereMesh].InitSphere(m_renderer.GetDevice(), 0.5f, 32, 16);	//球の形(半径0.5)
}

void Engine::Run(Game& game)
{
	game.Init(*this);						//ゲームの準備
	m_window.Show(m_nCmdShow);				//準備が終わってから表示

	using Clock = std::chrono::steady_clock;
	Clock::time_point prev = Clock::now();

	while (m_window.ProcessMessages())		//falseで終了
	{
		Clock::time_point now = Clock::now();
		float dt = std::chrono::duration<float>(now - prev).count();	//前フレームからの秒
		prev = now;

		m_input.Update();					//このフレームのキー状態

		UpdateObjects(dt);					//GameObjectを動かす
		game.Update(*this, dt);				//ゲーム全体のルール

		m_renderer.BeginFrame();
		DrawModels();						//3D(自動)
		game.Draw(*this);					//その場で描く分
		DrawSprites();						//2D(3Dより手前)
		m_renderer.BeginUI();
		game.DrawUI(*this);					//ImGui
		m_renderer.EndUI();
		m_renderer.EndFrame();
	}
}

//画像を読み込んで、番号を返す
int Engine::LoadTexture(const wchar_t* file)
{
	m_textures.emplace_back();										//末尾に1枚分追加
	m_textures.back().Init(m_renderer.GetGraphicsDevice(), file);	//読み込み+GPUへ転送
	return (int)m_textures.size() - 1;								//追加した場所=番号
}

//モデルファイルを読んで、形の番号を返す(失敗は-1)
int Engine::LoadMesh(const char* file)
{
	std::vector<Vertex> vertices;
	std::vector<UINT> indices;
	if (!ModelLoader::Load(file, vertices, indices)) return -1;	//理由はログ

	int mesh = AddMesh();
	m_meshes[mesh].Init(m_renderer.GetDevice(), vertices.data(), (UINT)vertices.size(),
		indices.data(), (UINT)indices.size());						//GPUへ
	return mesh;
}

Model* Engine::CreateModel(int mesh, int texture)
{
	return AddModel(mesh, texture);
}

Model* Engine::CreateTriangle(int texture)
{
	return AddModel(m_triangleMesh, texture);
}

Model* Engine::CreateSphere(int texture)
{
	return AddModel(m_sphereMesh, texture);
}

Sprite* Engine::CreateSprite(int texture, const Vector2& position, const Vector2& size)
{
	m_sprites.push_back(std::make_unique<Sprite>());	//本体はヒープに置く(移動しない)
	Sprite* sprite = m_sprites.back().get();
	sprite->texture = texture;
	sprite->position = position;
	sprite->size = size;
	return sprite;
}

void Engine::Destroy(Model* model)
{
	std::erase_if(m_models, [model](const std::unique_ptr<Model>& m) { return m.get() == model; });	//一致したら消す(deleteも自動)
}

void Engine::Destroy(Sprite* sprite)
{
	std::erase_if(m_sprites, [sprite](const std::unique_ptr<Sprite>& s) { return s.get() == sprite; });
}

void Engine::UpdateObjects(float dt)
{
	for (std::unique_ptr<GameObject>& obj : m_pendingObjects)
	{
		m_objects.push_back(std::move(obj));			//出現待ちを本番の一覧へ
	}
	m_pendingObjects.clear();

	for (std::unique_ptr<GameObject>& obj : m_objects)
	{
		obj->Update(dt);								//中でSpawnしてもm_pendingObjectsに入るので安全
	}

	std::erase_if(m_objects, [](const std::unique_ptr<GameObject>& obj) { return !obj->IsAlive(); });	//Destroy予約を消す
}

int Engine::AddMesh()
{
	m_meshes.emplace_back();				//末尾に1個分追加
	return (int)m_meshes.size() - 1;		//追加した場所=番号
}

Model* Engine::AddModel(int mesh, int texture)
{
	m_models.push_back(std::make_unique<Model>());	//本体はヒープに置く(移動しない)
	Model* model = m_models.back().get();
	model->m_mesh = mesh;					//friendなので設定できる
	model->texture = texture;
	return model;
}

void Engine::DrawModels()
{
	for (const std::unique_ptr<Model>& m : m_models)
	{
		if (!m->visible || m->m_mesh < 0) continue;	//読み込み失敗の形は飛ばす
		DrawMesh(m->m_mesh, m->texture, m->transform, m->color);
	}
}

void Engine::DrawSprites()
{
	for (const std::unique_ptr<Sprite>& s : m_sprites)
	{
		if (!s->visible) continue;
		DrawSprite(s->position, s->size, s->rotation, s->texture, s->color);
	}
}

void Engine::DrawMesh(int mesh, int texture, const Transform& transform, const Vector4& color)
{
	m_renderer.Draw(m_pipeline, m_meshes[mesh], m_textures[texture], transform, color);
}

void Engine::DrawSprite(const Vector2& position, const Vector2& size, float rotation, int texture, const Vector4& color)
{
	Transform t;
	t.position.x = position.x;				//中心のピクセル位置
	t.position.y = position.y;
	t.scale.x = size.x;						//1x1の板を、ピクセルサイズに拡大
	t.scale.y = size.y;
	t.rotation.z = rotation;				//画面の回転(Z軸まわり)
	m_renderer.DrawSprite(m_pipeline, m_meshes[m_quadMesh], m_textures[texture], t, color);
}
