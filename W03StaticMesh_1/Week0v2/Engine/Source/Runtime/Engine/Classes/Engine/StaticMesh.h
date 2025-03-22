#pragma once
#include "Define.h"
#include "D3D11RHI/GraphicDevice.h"
struct FMaterialSubset
{
    uint32 StartIndex; // 해당 재질이 적용되는 인덱스 버퍼의 시작 위치
    uint32 IndexCount; // 이 재질에 해당하는 삼각형 인덱스의 총 개수
    uint32 MaterialIndex; // 재질 배열에서의 인덱스
};

class FStaticMesh {
public:
    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11Buffer* indexBuffer = nullptr;
    UINT numVertices = 0;
    UINT numIndices = 0;
    std::unique_ptr<FVertexSimple[]> vertices;
    std::unique_ptr<UINT[]> indices;
    TArray<FMaterialSubset> materialSubsets;

    FStaticMesh(ID3D11Buffer* vb, UINT vCount, const FVertexSimple* vertexData,
        ID3D11Buffer* ib = nullptr, UINT iCount = 0, const UINT* indexData = nullptr)
        : vertexBuffer(vb), numVertices(vCount), indexBuffer(ib), numIndices(iCount) {

        if (vertexData) {
            vertices = std::make_unique<FVertexSimple[]>(vCount);
            std::memcpy(vertices.get(), vertexData, vCount * sizeof(FVertexSimple));
            UE_LOG(LogLevel::Display, "cpy");
        }

        if (indexData && iCount > 0) {
            indices = std::make_unique<UINT[]>(iCount);
            std::memcpy(indices.get(), indexData, iCount * sizeof(UINT));
        }
    }
    ~FStaticMesh() {
        if (vertexBuffer) {
            vertexBuffer->Release();
            vertexBuffer = nullptr;
        }
        if (indexBuffer) {
            indexBuffer->Release();
            indexBuffer = nullptr;
        }
    }
};