#include "LightComponent.h"
#include "UBillboardComponent.h"
#include "Math/JungleMath.h"
#include "UnrealEd/PrimitiveBatch.h"

ULightComponentBase::ULightComponentBase()
{
    // FString name = "SpotLight";
    // SetName(name);
    InitializeLight();
}

ULightComponentBase::~ULightComponentBase()
{
    delete texture2D;
}
void ULightComponentBase::SetColor(FVector4 newColor)
{
    color = newColor;
}

FVector4 ULightComponentBase::GetColor() const
{
    return color;
}

float ULightComponentBase::GetRadius() const
{
    return radius;
}

void ULightComponentBase::SetRadius(float r)
{
    radius = r;
}

void ULightComponentBase::Render()
{
    texture2D->Render();
    FMatrix Model = JungleMath::CreateModelMatrix(GetWorldLocation(), GetWorldRotation(), {1,1,1});
    //FMatrix MVP = Model * GetEngine().View * GetEngine().Projection;
    UPrimitiveBatch::GetInstance().AddCone(GetWorldLocation(), radius, 15, 140, color, Model);
    UPrimitiveBatch::GetInstance().RenderOBB(AABB, GetWorldLocation(), Model);
}

void ULightComponentBase::InitializeLight()
{
    texture2D = new UBillboardComponent();
    texture2D->SetTexture(L"Assets/Texture/spotLight.png");
    texture2D->InitializeComponent();
    AABB.max = { 1.f,1.f,0.1f };
    AABB.min = { -1.f,-1.f,-0.1f };
    color = { 1,1,1,1 };
    radius = 5;
}

void ULightComponentBase::TickComponent(float DeltaTime)
{
    texture2D->TickComponent(DeltaTime);
    texture2D->SetLocation(GetWorldLocation());

}

int ULightComponentBase::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    bool res =AABB.Intersect(rayOrigin, rayDirection, pfNearHitDistance);
    return res;
}

