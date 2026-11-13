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
	m_linePipeline.Init(m_renderer.GetDevice(), L"line.hlsl", m_renderer.GetFormat(), m_renderer.GetDepthFormat(),
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);	//線の描き方

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

		if (m_window.TakeResized() && !m_window.IsMinimized())
		{
			m_renderer.Resize(m_window.GetWidth(), m_window.GetHeight());	//画面サイズの資源を作り直す
		}
		if (m_window.IsMinimized())
		{
			Sleep(10);						//0x0では描けないので休む(CPUを使い切らない)
			continue;
		}

		m_input.Update(m_window);			//このフレームのキー・マウス状態

		UpdateObjects(dt);					//GameObjectを動かす
		game.Update(*this, dt);				//ゲーム全体のルール

		if (m_input.IsPressed(VK_F1)) m_debugCamera.enabled = !m_debugCamera.enabled;	//デバッグカメラ切り替え
		if (m_debugCamera.enabled) m_debugCamera.Update(m_debugView, m_input);
		m_renderer.SetCamera(m_debugCamera.enabled ? m_debugView : m_camera);			//使う方を渡す

		m_renderer.BeginFrame();
		DrawModels();						//3D(自動)
		game.Draw(*this);					//その場で描く分
		m_renderer.DrawLines(m_linePipeline, m_lineVertices);	//溜めた線をまとめて
		m_lineVertices.clear();				//次のフレームは空から
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
//
int Engine::LoadMesh(const char* file, float scale)
{
	std::vector<Vertex> vertices; //からの配列を2つ用意する　ModelLoaderの出力の受け皿　
	std::vector<UINT> indices; 
	if (!ModelLoader::Load(file, scale, vertices, indices)) return -1;	//理由はログ 

	int mesh = AddMesh(); //GPUに置く場所を確保する
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

void Engine::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color)
{
	Vertex v = {};
	v.color[0] = color.x;
	v.color[1] = color.y;
	v.color[2] = color.z;
	v.color[3] = color.w;

	v.position[0] = start.x;				//始点
	v.position[1] = start.y;
	v.position[2] = start.z;
	m_lineVertices.push_back(v);

	v.position[0] = end.x;					//終点(色は同じ)
	v.position[1] = end.y;
	v.position[2] = end.z;
	m_lineVertices.push_back(v);
}

void Engine::DrawGrid(float size, int divisions, const Vector4& color)
{
	const float half = size * 0.5f;
	const float step = size / divisions;					//1マスの幅
	for (int i = 0; i <= divisions; i++)					//端の線も含めるので+1本
	{
		float p = -half + step * i;
		DrawLine({ p, 0.0f, -half }, { p, 0.0f, half }, color);	//z方向の線(xをずらして並べる)
		DrawLine({ -half, 0.0f, p }, { half, 0.0f, p }, color);	//x方向の線(zをずらして並べる)
	}
}

void Engine::DrawBox(const Vector3& center, const Vector3& size, const Vector4& color)
{
	const Vector3 h = size * 0.5f;							//中心から各面までの距離
	Vector3 c[8];											//角8個(番号のビット: 1=+x, 2=+y, 4=+z)
	for (int i = 0; i < 8; i++)
	{
		c[i] = { center.x + ((i & 1) ? h.x : -h.x),
		         center.y + ((i & 2) ? h.y : -h.y),
		         center.z + ((i & 4) ? h.z : -h.z) };
	}

	const int edges[12][2] =								//辺12本(ビットが1つだけ違う角どうし)
	{
		{ 0, 1 }, { 2, 3 }, { 4, 5 }, { 6, 7 },				//x方向
		{ 0, 2 }, { 1, 3 }, { 4, 6 }, { 5, 7 },				//y方向
		{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },				//z方向
	};
	for (const auto& e : edges)
	{
		DrawLine(c[e[0]], c[e[1]], color);
	}
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
