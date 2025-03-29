cbuffer CB_HizCull : register(b0)
{
    matrix gView;
    matrix gProj;
    matrix gViewProj;
    float4 gFrustumPlanes[6];
    float3 gEyePos;
    float gViewportSize;
};

StructuredBuffer<float4> ColliderBuffer : register(t0);
Texture2D HiZMap : register(t1);
SamplerState samp : register(s0);
RWStructuredBuffer<float> CullingBuffer : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    float4 sphere = ColliderBuffer[index];
    
    // sphere center를 NDC로 투영
    float4 projPos = mul(float4(sphere.xyz, 1.0f), gViewProj);
    projPos /= projPos.w;
    
    // Occludee가 화면 상에 보장되는 2x2 texel 영역 산출 (계산 생략)
    // 간략화를 위해 mipLevel은 0으로 가정
    uint mipLevel = 0;
    
    float2 screenPos = (projPos.xy * 0.5f + 0.5f) * gViewportSize;
    uint2 texelCoord = uint2(screenPos);
    
    // 2x2 texel 샘플링하여 최대 depth 찾기
    float maxDepth = 0.0f;
    [unroll]
    for (int y = 0; y < 2; y++)
    {
        for (int x = 0; x < 2; x++)
        {
            float depth = HiZMap.Load(int3(texelCoord + uint2(x, y), mipLevel)).r;
            maxDepth = max(maxDepth, depth);
        }
    }
    
    // 투영된 depth와 비교하여 occlusion 판단
    float visible = (projPos.z <= maxDepth) ? 1.0f : 0.0f;
    CullingBuffer[index] = visible;
}