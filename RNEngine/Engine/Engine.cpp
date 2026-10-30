#include "Engine/Engine.h"
#include "Engine/Game.h"
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

		game.Update(*this, dt);				//動かす

		m_renderer.BeginFrame();
		game.Draw(*this);					//3D・スプライト
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

//球の形を作って、番号を返す
int Engine::CreateSphere(float radius, UINT slices, UINT stacks)
{
	int index = AddMesh();
	m_meshes[index].InitSphere(m_renderer.GetDevice(), radius, slices, stacks);
	return index;
}

int Engine::AddMesh()
{
	m_meshes.emplace_back();				//末尾に1個分追加
	return (int)m_meshes.size() - 1;		//追加した場所=番号
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
