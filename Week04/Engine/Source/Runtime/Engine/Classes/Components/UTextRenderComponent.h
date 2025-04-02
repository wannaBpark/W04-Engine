#pragma once
#include "UBillboardComponent.h"
#include "PrimitiveComponent.h"

class UTextRenderComponent : public UPrimitiveComponent {
    DECLARE_CLASS(UTextRenderComponent, UPrimitiveComponent);

public:
    UTextRenderComponent();
    virtual ~UTextRenderComponent() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

    void ClearText();
    void SetText(FWString _text);
    FWString GetText() { return text; }
    void SetRowColumnCount(int _cellsPerRow, int _cellsPerColumn);
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    bool CheckPickingOnNDC(const TArray<FVector>& checkQuad, float& hitDistance);
    void SetTexture(FWString _fileName);
    ID3D11Buffer* vertexTextBuffer;
    TArray<FVertexTexture> vertexTextureArr;
    UINT numTextVertices;
    std::shared_ptr<FTexture> Texture;
    FMatrix CreateTextMatrix();
protected:
    USceneComponent* m_parent = nullptr;
    FWString text;
    TArray<FVector> quad;

    const int quadSize = 2;

    int RowCount;
    int ColumnCount;

    float quadWidth = 2.0f;
    float quadHeight = 2.0f;

    void setStartUV(char alphabet, float& outStartU, float& outStartV);
    void setStartUV(wchar_t hangul, float& outStartU, float& outStartV);
    void CreateTextTextureVertexBuffer(const TArray<FVertexTexture>& _vertex, UINT byteWidth);
    

    void TextMVPRendering();
};