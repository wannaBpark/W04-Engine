#pragma once
// SoftwareOcclusionCulling.h

#pragma once
#include <vector>
#include <algorithm>
#include <cmath>
#include <cfloat>

#include "Define.h"
#include "Container/Array.h"
#include "Container/Set.h"


class KDTreeSystem;
class UPrimitiveComponent;
class FViewportCameraTransform;

// 간단한 Software Z-Buffer 클래스 (낮은 해상도)
class SoftwareZBuffer
{
public:
    int width, height;
    std::vector<float> zBuffer;

    SoftwareZBuffer(int w, int h) : width(w), height(h)
    {
        zBuffer.resize(width * height, 1.0f);
    }

    void Clear()
    {
        std::fill(zBuffer.begin(), zBuffer.end(), 1.0f);
    }

    // 매우 단순화된 rasterize 함수: 
    // 입력으로 주어진 bounding box의 화면 좌표 범위 내에서, 깊이값을 업데이트
    // 실제로는 오브젝트의 삼각형들을 rasterize해야 하지만 여기서는 바운딩 박스 중심의 깊이를 사용합니다.
    void RasterizeOccluder(const FBoundingBox& bbox, const FMatrix& mvp);

    void PerformSWOcclusionCulling(KDTreeSystem*& kdTree, const FFrustum& frustum, SoftwareZBuffer& swZBuffer, TArray<UPrimitiveComponent*>& OutVisibleComponents);

    // 화면 좌표 (x, y)에서 깊이값 반환
    float SampleDepth(int x, int y) const
    {
        return zBuffer[y * width + x];
    }
};

