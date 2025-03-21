#pragma once
#include "Engine/Source/Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Engine/Source/Editor/PropertyEditor/ShowFlags.h"

class UPrimitiveComponent : public USceneComponent
{
    DECLARE_CLASS(UPrimitiveComponent, USceneComponent)

public:
    UPrimitiveComponent();
    virtual ~UPrimitiveComponent() override;

    virtual void Initialize() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual void Release() override;
    virtual void Render() override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance);
    bool IntersectRayTriangle(
        const FVector& rayOrigin, const FVector& rayDirection,
        const FVector& v0, const FVector& v1, const FVector& v2, float& hitDistance
    );
    FBoundingBox AABB;

private:
    FString m_Type;

protected:
    std::shared_ptr<FStaticMesh> staticMesh;

public:
    FString GetType() { return m_Type; }

    void SetType(const FString& _Type)
    {
        m_Type = _Type;
        staticMesh = FEngineLoop::resourceMgr.GetMesh(m_Type);
    }
};
