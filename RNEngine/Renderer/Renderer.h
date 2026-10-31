#pragma once
#include "Common.h"
#include "GPU/GraphicsDevice.h"
#include "GPU/SwapChain.h"
#include "GPU/DepthBuffer.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Mesh.h"
#include "UI/ImGuiLayer.h"
#include "Math/Transform.h"
#include "Renderer/Texture.h"
#include "Math/Vector.h"
#include "Renderer/Camera.h"


//何をどう描くか指示する
class Renderer
{
public:
	~Renderer();

	void Init(HWND hwnd, UINT width, UINT height);
	void BeginFrame();									//記録開始+画面クリア
	void Draw(const Pipeline& pipeline, const Mesh& mesh, const Texture& texture, const Transform& transform, const Vector4& color);	//描画命令を追加
	void DrawSprite(const Pipeline& pipeline, const Mesh& mesh, const Texture& texture,const Transform& transform, const Vector4& color);	//ピクセル座標で描く
	void EndFrame();									//送信+表示+GPU待ち

	void BeginUI();										//UI記録開始
	void EndUI();
	void SetCamera(const Camera& camera) { m_camera = camera; }	//このフレームで使うカメラ(値をコピー)


	ID3D12Device* GetDevice() const { return m_gfx.GetDevice(); }
	DXGI_FORMAT GetFormat() const { return m_swapChain.GetFormat(); }
	DXGI_FORMAT GetDepthFormat() const { return m_depthBuffer.GetFormat(); }
	GraphicsDevice& GetGraphicsDevice() { return m_gfx; }	//Texture作成用

private:
	static void Transition(ID3D12GraphicsCommandList* cmd, ID3D12Resource* resource,D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);	//バリア
	DirectX::XMFLOAT4X4 m_orthoProjection;		//平行投影行列(ピクセル→画面)

	GraphicsDevice m_gfx;						//先に宣言(後に破棄)
	SwapChain m_swapChain;
	DepthBuffer m_depthBuffer;					//前後関係の判定用
	D3D12_VIEWPORT m_viewport = {};				//描画範囲
	D3D12_RECT m_scissorRect = {};				//切り抜き範囲
	ImGuiLayer m_imgui;

	Camera m_camera;							//3D用のカメラ
	float m_aspect = 1.0f;						//画面の幅÷高さ


};