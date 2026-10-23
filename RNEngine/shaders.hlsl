struct VSInput								//頂点シェーダー入力
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct PSInput								//ピクセルシェーダー入力
{
    float4 pos : SV_POSITION; //画面上の位置
    float4 color : COLOR;
};

cbuffer Transform : register(b0) //ルート定数
{
    row_major float4x4 world; //ワールド行列
};

PSInput VSMain(VSInput input)				//頂点ごとに実行
{
    PSInput output;
    output.pos = mul(float4(input.pos, 1.0f), world); //ローカル→ワールド    
    output.color = input.color;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET //ピクセルごとに実行
{
    return input.color; //色をそのまま出力
}
