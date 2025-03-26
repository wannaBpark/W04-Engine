#include "TransformGizmo.h"
#include "GizmoArrowComponent.h"
#include "Define.h"
#include "UObject/ObjectFactory.h"
#include "GizmoCircleComponent.h"
#include "Components/Player.h"
#include "GizmoRectangleComponent.h"
#include "World.h"
#include "Engine/FLoaderOBJ.h"

UTransformGizmo::UTransformGizmo()
{
    FManagerOBJ::CreateStaticMesh("Assets/gizmo_loc_x.obj");
    FManagerOBJ::CreateStaticMesh("Assets/gizmo_loc_y.obj");
    FManagerOBJ::CreateStaticMesh("Assets/gizmo_loc_z.obj");
    FManagerOBJ::CreateStaticMesh("Assets/gizmo_rot_x.obj");
    FManagerOBJ::CreateStaticMesh("Assets/gizmo_rot_y.obj");
    FManagerOBJ::CreateStaticMesh("Assets/gizmo_rot_z.obj");
    FManagerOBJ::CreateStaticMesh("Assets/gizmo_scale_x.obj");
    FManagerOBJ::CreateStaticMesh("Assets/gizmo_scale_y.obj");
    FManagerOBJ::CreateStaticMesh("Assets/gizmo_scale_z.obj");
    
    UGizmoArrowComponent* locationX = FObjectFactory::ConstructObject<UGizmoArrowComponent>();
    locationX->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"gizmo_loc_x.obj"));
	locationX->SetParent(this);
    locationX->SetGizmoType(UGizmoBaseComponent::ArrowX);
	AttachChildren.Add(locationX);
	ArrowArr.Add(locationX);
    GetWorld()->GetObjectArr().Add(locationX);

    UGizmoArrowComponent* locationY = FObjectFactory::ConstructObject<UGizmoArrowComponent>();
    locationY->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"gizmo_loc_y.obj"));
    locationY->SetParent(this);
    locationY->SetGizmoType(UGizmoBaseComponent::ArrowY);
    AttachChildren.Add(locationY);
    ArrowArr.Add(locationY);
    GetWorld()->GetObjectArr().Add(locationY);

    UGizmoArrowComponent* locationZ = FObjectFactory::ConstructObject<UGizmoArrowComponent>();
    locationZ->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"gizmo_loc_z.obj"));
    locationZ->SetParent(this);
    locationZ->SetGizmoType(UGizmoBaseComponent::ArrowZ);
    AttachChildren.Add(locationZ);
    ArrowArr.Add(locationZ);
    GetWorld()->GetObjectArr().Add(locationZ);

    UGizmoRectangleComponent* ScaleX = FObjectFactory::ConstructObject<UGizmoRectangleComponent>();
    ScaleX->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"gizmo_scale_x.obj"));
    ScaleX->SetParent(this);
    ScaleX->SetGizmoType(UGizmoBaseComponent::ScaleX);
    AttachChildren.Add(ScaleX);
    RectangleArr.Add(ScaleX);
    GetWorld()->GetObjectArr().Add(ScaleX);

    UGizmoRectangleComponent* ScaleY = FObjectFactory::ConstructObject<UGizmoRectangleComponent>();
    ScaleY->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"gizmo_scale_y.obj"));
    ScaleY->SetParent(this);
    ScaleY->SetGizmoType(UGizmoBaseComponent::ScaleY);
    AttachChildren.Add(ScaleY);
    RectangleArr.Add(ScaleY);
    GetWorld()->GetObjectArr().Add(ScaleY);

    UGizmoRectangleComponent* ScaleZ = FObjectFactory::ConstructObject<UGizmoRectangleComponent>();
    ScaleZ->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"gizmo_scale_z.obj"));
    ScaleZ->SetParent(this);
    ScaleZ->SetGizmoType(UGizmoBaseComponent::ScaleZ);
    AttachChildren.Add(ScaleZ);
    RectangleArr.Add(ScaleZ);
    GetWorld()->GetObjectArr().Add(ScaleZ);

    UGizmoCircleComponent* CircleX = FObjectFactory::ConstructObject<UGizmoCircleComponent>();
    CircleX->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"gizmo_rot_x.obj"));
    CircleX->SetParent(this);
    CircleX->SetGizmoType(UGizmoBaseComponent::CircleX);
    AttachChildren.Add(CircleX);
    CircleArr.Add(CircleX);
    GetWorld()->GetObjectArr().Add(CircleX);

    UGizmoCircleComponent* CircleY = FObjectFactory::ConstructObject<UGizmoCircleComponent>();
    CircleY->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"gizmo_rot_y.obj"));
    CircleY->SetParent(this);
    CircleY->SetGizmoType(UGizmoBaseComponent::CircleY);
    AttachChildren.Add(CircleY);
    CircleArr.Add(CircleY);
    GetWorld()->GetObjectArr().Add(CircleY);

    UGizmoCircleComponent* CircleZ = FObjectFactory::ConstructObject<UGizmoCircleComponent>();
    CircleZ->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"gizmo_rot_z.obj"));
    CircleZ->SetParent(this);
    CircleZ->SetGizmoType(UGizmoBaseComponent::CircleZ);
    AttachChildren.Add(CircleZ);
    CircleArr.Add(CircleZ);
    GetWorld()->GetObjectArr().Add(CircleZ);
}

UTransformGizmo::~UTransformGizmo()
{

}

void UTransformGizmo::Initialize()
{
	Super::Initialize();
}

void UTransformGizmo::Update(double deltaTime)
{
	Super::Update(deltaTime);
	if (GetWorld()->GetPickingObj()) {
		SetLocation(GetWorld()->GetPickingObj()->GetWorldLocation());
	if (GetWorld()->GetPlayer()->GetCoordiMode() == CoordiMode::CDM_LOCAL)
	{
			SetRotation(GetWorld()->GetPickingObj()->GetQuat());
	}
	else if (GetWorld()->GetPlayer()->GetCoordiMode() == CoordiMode::CDM_WORLD)
			SetRotation(FVector(0.0f,0.0f,0.0f));
	}
	for (int i = 0;i < 3;i++)
	{
		ArrowArr[i]->Update(deltaTime);
		CircleArr[i]->Update(deltaTime);
		RectangleArr[i]->Update(deltaTime);
	}
}

void UTransformGizmo::Release()
{
}

void UTransformGizmo::Render()
{
}
