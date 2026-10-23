#include "Renderer/Pipeline.h"

void Pipeline::Init(ID3D12Device* device, const wchar_t* shaderFile, DXGI_FORMAT rtvFormat)
{
	//ルートシグネチャ(空)
	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;	//頂点バッファ使用
	ComPtr<ID3DBlob> rsBlob, rsError;
	HRESULT hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rsBlob, &rsError);
	if (rsError)
	{
		OutputDebugStringA((const char*)rsError->GetBufferPointer());
	}
	CheckHR(hr);
	CheckHR(device->CreateRootSignature(0, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)));

	//シェーダー
	ComPtr<ID3DBlob> vs = CompileShader(shaderFile, "VSMain", "vs_5_0");
	ComPtr<ID3DBlob> ps = CompileShader(shaderFile, "PSMain", "ps_5_0");

	//入力レイアウト(Mesh.hのVertexと一致させる)
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	//PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
	psoDesc.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
	psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = rtvFormat;					//SwapChainと一致
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleMask = UINT_MAX;

	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;		//塗りつぶし
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;		//裏面を描かない
	psoDesc.RasterizerState.FrontCounterClockwise = FALSE;			//時計回りが表
	psoDesc.RasterizerState.DepthClipEnable = TRUE;

	D3D12_RENDER_TARGET_BLEND_DESC& blend = psoDesc.BlendState.RenderTarget[0];
	blend.BlendEnable = FALSE;						//ブレンドなし
	blend.SrcBlend = D3D12_BLEND_ONE;
	blend.DestBlend = D3D12_BLEND_ZERO;
	blend.BlendOp = D3D12_BLEND_OP_ADD;
	blend.SrcBlendAlpha = D3D12_BLEND_ONE;
	blend.DestBlendAlpha = D3D12_BLEND_ZERO;
	blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blend.LogicOp = D3D12_LOGIC_OP_NOOP;
	blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	//0だと何も描かれない

	psoDesc.DepthStencilState.DepthEnable = FALSE;		//深度なし
	psoDesc.DepthStencilState.StencilEnable = FALSE;

	CheckHR(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

void Pipeline::Set(ID3D12GraphicsCommandList* cmd) const
{
	cmd->SetPipelineState(m_pipelineState.Get());
	cmd->SetGraphicsRootSignature(m_rootSignature.Get());
}

ComPtr<ID3DBlob> Pipeline::CompileShader(const wchar_t* file, const char* entry, const char* target)
{
	UINT flags = 0;
#ifdef _DEBUG
	flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;	//デバッグ用
#endif
	ComPtr<ID3DBlob> blob, error;
	HRESULT hr = D3DCompileFromFile(file, nullptr, nullptr, entry, target, flags, 0, &blob, &error);
	if (error)
	{
		OutputDebugStringA((const char*)error->GetBufferPointer());	//HLSLエラー表示
	}
	CheckHR(hr);
	return blob;
}