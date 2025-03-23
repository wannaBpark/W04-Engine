Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

// MatrixBuffer: 변환 행렬 관리
cbuffer MatrixBuffer : register(b0)
{
    row_major float4x4 MVP;
    float4 UUID;
};

cbuffer LightingBuffer : register(b1)
{
    float3 LightDirection; // 조명 방향 (단위 벡터; 빛이 들어오는 방향의 반대 사용)
    float LightPad0; // 16바이트 정렬용 패딩
    float3 LightColor; // 조명 색상 (예: (1, 1, 1))
    float LightPad1; // 16바이트 정렬용 패딩
    float AmbientFactor; // ambient 계수 (예: 0.1)
    float3 LightPad2; // 16바이트 정렬 맞춤 추가 패딩
};

cbuffer MaterialBuffer : register(b2)
{
    float4 DiffuseColor;
    float4 AmbientColor;
    float4 SpecularColor;
    float SpecularPower;
    float3 MaterialPad0;
}

cbuffer FlagBuffer : register(b3)
{
    bool IsSelect;
    bool IsLit;
    float2 flagPad0;
}

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}