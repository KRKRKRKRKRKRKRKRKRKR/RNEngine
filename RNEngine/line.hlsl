struct VSInput                               //Vertexと同じ並び(uvは使わない)
{
    float3 pos : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

cbuffer Object : register(b0) //ルート定数(shaders.hlslと同じ形)
{
    row_major float4x4 viewProj; //ワールド→画面(線は最初からワールド座標)
    float4 tint; //全体に掛ける色
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.pos = mul(float4(input.pos, 1.0f), viewProj);
    output.color = input.color * tint;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color; //頂点の色だけ(画像なし)
}
