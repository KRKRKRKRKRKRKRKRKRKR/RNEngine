struct VSInput
{
    float3 pos : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

cbuffer Object : register(b0) //ルート定数
{
    row_major float4x4 world; //ワールド行列
    float4 tint; //色
};

Texture2D tex : register(t0); //画像
SamplerState samp : register(s0); //読み方


PSInput VSMain(VSInput input)				//頂点ごとに実行
{
    PSInput output;
    output.pos = mul(float4(input.pos, 1.0f), world); //ローカル→ワールド    
    output.color = input.color;
    output.uv = input.uv;

    return output;
}

float4 PSMain(PSInput input) : SV_TARGET //ピクセルごとに実行
{
    return tex.Sample(samp, input.uv) * tint; //画像×色
}
