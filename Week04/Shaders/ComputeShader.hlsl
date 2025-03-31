cbuffer CB_OcclusionCulling : register(b0)
{
    float4x4 viewProj; // 카메라의 view * proj 행렬 (row-major)
    float viewportWidth;
    float viewportHeight;
    uint occludeeCount; // occludee 개수를 전달 (추가)
};

struct OccludeeData
{
    float centerX;
    float centerY;
    float centerZ;
    float radius;
};

// occludee 데이터가 저장된 구조화 버퍼 (t0)
StructuredBuffer<OccludeeData> gOccludeeBuffer : register(t0);

// 계층적 Z 버퍼 (Hi-Z): 각 mip level에 해당하는 SRV
Texture2D gHiZTexture : register(t1);
SamplerState gHiZSampler : register(s0);

// 결과 버퍼: 각 occludee별로 1 (visible) 또는 0 (occluded)을 저장 (u0)
RWStructuredBuffer<int> gResultBuffer : register(u0);

[numthreads(64, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    // occludee 데이터 개수 검사
    if (index >= occludeeCount)
        return;
    
    OccludeeData data = gOccludeeBuffer[index];
    float4 center = float4(data.centerX, data.centerY, data.centerZ, 1.0f);

    // viewProj 변환
    float4 clipPos = mul(center, viewProj);
    // Perspective divide
    if (abs(clipPos.w) > 1e-6)
    {
        clipPos.xyz /= clipPos.w;
    }
    
    // NDC [-1,1] -> 화면 좌표 [0, viewportWidth], [0, viewportHeight]
    float screenX = (clipPos.x * 0.5f + 0.5f) * viewportWidth;
    float screenY = (-clipPos.y * 0.5f + 0.5f) * viewportHeight;

    // 화면 좌표를 int로 변환 (픽셀 좌표)
    int2 screenPos = int2(screenX, screenY);

    // Hi-Z mip level 선택: 여기서는 0번 mip level (최고 해상도) 사용
    float hiZDepth = gHiZTexture.Load(int3(screenPos, 0)).r;

    // 가시성 판정: occludee의 clipPos.z와 hiZDepth 비교 (clipPos.z가 작으면 더 앞)
    int visible = (clipPos.z < hiZDepth) ? 1 : 0;
    gResultBuffer[index] = visible;
}
