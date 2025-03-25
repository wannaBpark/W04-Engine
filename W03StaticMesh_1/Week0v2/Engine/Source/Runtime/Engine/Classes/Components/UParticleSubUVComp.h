#pragma once
#include "UBillboardComponent.h"

class UParticleSubUVComp : public UBillboardComponent
{
    DECLARE_CLASS(UParticleSubUVComp, UBillboardComponent)

public:
    UParticleSubUVComp();
    virtual ~UParticleSubUVComp() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual void Render() override;

    void SetRowColumnCount(int _cellsPerRow, int _cellsPerColumn);

    ID3D11Buffer* vertexSubUVBuffer;
    UINT numTextVertices;
private:

    int indexU = 0;
    int indexV = 0;
    float second = 0;

    int CellsPerRow;
    int CellsPerColumn;



    // TODO: UActorComponent에 Activate, Deactivate 만들기
    bool bIsActivate = true;

    void UpdateVertexBuffer(const TArray<FVertexTexture>& vertices);
    void CreateSubUVVertexBuffer();
};
