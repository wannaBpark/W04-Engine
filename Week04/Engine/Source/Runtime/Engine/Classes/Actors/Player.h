#pragma once
#include "GameFramework/Actor.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ObjectTypes.h"
class UGizmoBaseComponent;
class UGizmoArrowComponent;
class USceneComponent;
class UPrimitiveComponent;


class AEditorPlayer : public AActor
{
    DECLARE_CLASS(AEditorPlayer, AActor)

public:
    struct FPickingTimeInfo
    {
        std::atomic<uint32> LastPickingTime = 0;
        std::atomic<uint32> NumAttempts = 0;
        std::atomic<uint64> AccumulatedTime = 0;

        FPickingTimeInfo()
        {
            assert(LastPickingTime.is_lock_free());
            assert(NumAttempts.is_lock_free());
            assert(AccumulatedTime.is_lock_free());
        }
    };

    static FPickingTimeInfo PickingTimeInfo;

public:
    AEditorPlayer() = default;

    virtual void Tick(float DeltaTime) override;

    void Input();
    bool PickGizmo(const FVector& rayOrigin);
    void PickActor(const FVector& pickPosition);
    void AddControlMode();
    void AddCoordiMode();

public:
    void SetMode(ControlMode InMode) { cMode = InMode; }
    ControlMode GetControlMode() const { return cMode; }
    CoordiMode GetCoordiMode() const { return cdMode; }

private:
    int RayIntersectsObject(const FVector& pickPosition, USceneComponent* obj, float& hitDistance, int& intersectCount);
    void ScreenToViewSpace(int screenX, int screenY, const FMatrix& viewMatrix, const FMatrix& projectionMatrix, FVector& rayOrigin);
    void PickedObjControl();
    void ControlRotation(USceneComponent* pObj, const UGizmoBaseComponent* Gizmo, int32 deltaX, int32 deltaY);
    void ControlTranslation(USceneComponent* pObj, const UGizmoBaseComponent* Gizmo, int32 deltaX, int32 deltaY);
    void ControlScale(USceneComponent* pObj, const UGizmoBaseComponent* Gizmo, int32 deltaX, int32 deltaY);
    bool bLeftMouseDown = false;
    bool bRightMouseDown = false;
    bool bSpaceDown = false;

    POINT m_LastMousePos;
    ControlMode cMode = CM_TRANSLATION;
    CoordiMode cdMode = CDM_WORLD;
};
