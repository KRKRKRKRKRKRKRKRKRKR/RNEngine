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

PSInput VSMain(VSInput input)				//頂点ごとに実行
{
    PSInput output;
    output.pos = float4(input.pos, 1.0f); //xyz→xyzw
    output.color = input.color;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET //ピクセルごとに実行
{
    return input.color; //色をそのまま出力
}