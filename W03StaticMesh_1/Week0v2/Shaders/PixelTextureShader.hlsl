Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer SubUVConstant : register(b1)
{
    float indexU;
    float indexV;
}

struct PSInput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET {
    float2 uv = input.texCoord + float2(indexU, indexV);
    //float4 col = gTexture.Sample(gSampler, input.texCoord);
    float4 col = gTexture.Sample(gSampler, uv);
    float threshold = 0.05; // 필요한 경우 임계값을 조정
    if (col.r < threshold && col.g < threshold && col.b < threshold)
        clip(-1); // 픽셀 버리기
    
    return col;
}
