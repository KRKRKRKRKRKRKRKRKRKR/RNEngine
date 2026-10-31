#include "Renderer/Pipeline.h"

void Pipeline::Init(ID3D12Device* device, const wchar_t* shaderFile, DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat,
	D3D12_PRIMITIVE_TOPOLOGY_TYPE topology)
{

	//ルートパラメータ
	D3D12_ROOT_PARAMETER params[2] = {};

	//[0] 行列(定数バッファ)
	params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	params[0].Constants.ShaderRegister = 0;		//b0
	params[0].Constants.RegisterSpace = 0;
	params[0].Constants.Num32BitValues = 16;	//float4x4=16個
	params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;	//頂点シェーダーのみ
	params[0].Constants.Num32BitValues = 20;	//float4x4(16)+色(4)
	params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	//頂点でも色(ピクセル)でも使う

	//[1] テクスチャ(SRV)
	D3D12_DESCRIPTOR_RANGE range = {};
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.NumDescriptors = 1;
	range.BaseShaderRegister = 0;				//t0
	range.RegisterSpace = 0;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params[1].DescriptorTable.NumDescriptorRanges = 1;
	params[1].DescriptorTable.pDescriptorRanges = &range;
	params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;		//ピクセルシェーダーのみ

	//サンプラー(画像の読み方)
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;			//なめらかに補間
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;			//範囲外は繰り返し
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;									//s0
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	//ルートシグネチャ
	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;	//頂点バッファ使用
	rsDesc.NumParameters = _countof(params);
	rsDesc.pParameters = params;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.pStaticSamplers = &sampler;
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

	//入力レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },	//12+16=28
	};


	//PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
	psoDesc.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
	psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
	psoDesc.PrimitiveTopologyType = topology;			//三角形 or 線
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

	psoDesc.DSVFormat = dsvFormat;									//DepthBufferと一致
	psoDesc.DepthStencilState.DepthEnable = TRUE;					//深度テストON
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;	//距離を書き込む
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;		//近い方だけ描く
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