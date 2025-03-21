#pragma once
#include "GameFramework/Actor.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ObjectTypes.h"

class UGizmoArrowComponent;
class USceneComponent;
class UPrimitiveComponent;

class UPlayer : public AActor
{
    DECLARE_CLASS(UPlayer, AActor)

    UPlayer();
    virtual ~UPlayer() override;

    // TODO: AActor에 맞게 수정하기
    // 지금 FObjectFactory에서 T->InitializeComponent()를 호출하고 있음
    virtual void InitializeComponent() {};
    virtual void Tick(float DeltaTime) override;

    // TODO: 삭제 해야함
    virtual void TickComponent(float DeltaTime) override;
    virtual void Release();

    void Input();
    bool PickGizmo(FVector& rayOrigin);
    void PickObj(FVector& pickPosition);
    void AddControlMode();
    void AddCoordiMode();
    void DeletePickedObj();

private:
    int RayIntersectsObject(const FVector& pickPosition, UPrimitiveComponent* obj, float& hitDistance, int& intersectCount);
    void ScreenToViewSpace(int screenX, int screenY, const FMatrix& viewMatrix, const FMatrix& projectionMatrix, FVector& rayOrigin);
    void PickedObjControl();
    void ControlRotation(USceneComponent* pObj, UPrimitiveComponent* Gizmo, int32 deltaX, int32 deltaY);
    void ControlTranslation(USceneComponent* pObj, UPrimitiveComponent* Gizmo, int32 deltaX, int32 deltaY);
    void ControlScale(USceneComponent* pObj, UPrimitiveComponent* Gizmo, int32 deltaX, int32 deltaY);
    bool bLeftMouseDown = false;
    bool bRightMouseDown = false;
    bool bSpaceDown = false;

    POINT m_LastMousePos;
    ControlMode cMode = CM_TRANSLATION;
    CoordiMode cdMode = CDM_WORLD;

public:
    void SetMode(ControlMode _Mode) { cMode = _Mode; }
    ControlMode GetControlMode() const { return cMode; }
    CoordiMode GetCoordiMode() const { return cdMode; }
};
