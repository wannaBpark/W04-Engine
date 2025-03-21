#pragma once
#include "UBillboardComponent.h"

class UText : public UBillboardComponent
{
    DECLARE_CLASS(UText, UBillboardComponent)

public:
    UText();
    virtual ~UText() override;

    virtual void Initialize() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual void Release() override;
    virtual void Render() override;
    void ClearText();
    void SetText(FWString _text);
    FWString GetText() { return text; }
    void SetRowColumnCount(int _cellsPerRow, int _cellsPerColumn);
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;

protected:
    ID3D11Buffer* vertexTextBuffer;
    FWString text;
    TArray<FVertexTexture> vertexTextureArr;
    UINT numTextVertices;

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
