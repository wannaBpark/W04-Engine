#pragma once
#include "Engine/Source/Runtime/CoreUObject/UObject/Object.h"
class UGizmoArrowComponent;
class USceneComponent;
class UPrimitiveComponent;
class UPlayer : public UObject
{
	DECLARE_CLASS(UTransformGizmo, UObject)

public:
	UPlayer();
	virtual				~UPlayer();

	virtual void		Initialize();
	virtual void		Update(double deltaTime);
	virtual void		Release();

	void				Input();
	bool				PickGizmo(FVector& rayOrigin);
	void				PickObj(FVector& pickPosition);
	void				AddControlMode();
	void				AddCoordiMode();
	void				DeletePickedObj();
private:
	int	 RayIntersectsObject(const FVector& pickPosition, UPrimitiveComponent* obj, float& hitDistance, int& intersectCount);
	void ScreenToViewSpace (int screenX, int screenY, const FMatrix& viewMatrix, const FMatrix& projectionMatrix, FVector& rayOrigin);
	void PickedObjControl();
	void ControlRotation( USceneComponent* pObj, UPrimitiveComponent* Gizmo, int32 deltaX, int32 deltaY);
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
	ControlMode GetControlMode() { return cMode; }
	CoordiMode GetCoordiMode() { return cdMode; }
};

