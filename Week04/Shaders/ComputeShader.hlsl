cbuffer CB_OcclusionCulling : register(b0)
{
    float4x4 viewProj;
    float viewportWidth;
    float viewportHeight;
    uint occludeeCount;
    float pad;
};

struct OccludeeData
{
    float centerX;
    float centerY;
    float centerZ;
    float radius;
};

StructuredBuffer<OccludeeData> gOccludeeBuffer : register(t0);
Texture2D gHiZTexture : register(t1);
SamplerState gHiZSampler : register(s0);
RWStructuredBuffer<int> gResultBuffer : register(u0);

[numthreads(64, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= occludeeCount)
        return;

    OccludeeData data = gOccludeeBuffer[index];
    float4 center = float4(data.centerX, data.centerY, data.centerZ, 1.0f);

    // Transform to clip space
    float4 clipPos = mul(center, viewProj);

    // Clip space -> NDC -> Screen space conversion
    if (abs(clipPos.w) > 1e-6)
    {
        clipPos.xyz /= clipPos.w;
    }

    // NDC [-1,1] -> Screen space [0, width/height]
    float screenX = (clipPos.x * 0.5f + 0.5f) * viewportWidth;
    float screenY = (1.0f - (clipPos.y * 0.5f + 0.5f)) * viewportHeight;

    float screenRadius = data.radius / clipPos.w * viewportWidth;

    float2 uv = float2(screenX / viewportWidth, screenY / viewportHeight);

    // Depth comparison (NDC to [0, 1] range)
    float objectDepth = clipPos.z * 0.5f + 0.5f;
    float hiZDepth = gHiZTexture.SampleLevel(gHiZSampler, uv, 0).r;

    int visible = (objectDepth - data.radius * 0.01) < hiZDepth ? 1 : 0;
    gResultBuffer[index] = visible;
}