#pragma once
#include "Define.h"
#include "D3D11RHI/GraphicDevice.h"
class FStaticMesh {
public:
    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11Buffer* indexBuffer = nullptr;
    UINT numVertices = 0;
    UINT numIndices = 0;
    std::unique_ptr<FVertexSimple[]> vertices;
    std::unique_ptr<UINT[]> indices;
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