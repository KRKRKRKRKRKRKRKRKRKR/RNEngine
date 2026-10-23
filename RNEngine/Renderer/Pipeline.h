#pragma once
#include "Common.h"

//描き方(シェーダー・描画設定)を決める
class Pipeline
{
public:
	void Init(ID3D12Device* device, const wchar_t* shaderFile, DXGI_FORMAT rtvFormat);
	void Set(ID3D12GraphicsCommandList* cmd) const;	//PSO+ルートシグネチャをセット

private:
	static ComPtr<ID3DBlob> CompileShader(const wchar_t* file, const char* entry, const char* target);

	ComPtr<ID3D12RootSignature> m_rootSignature;	//シェーダー入力の一覧
	ComPtr<ID3D12PipelineState> m_pipelineState;	//描画設定一式
};