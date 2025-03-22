#include "TransformGizmo.h"
#include "GizmoArrowComponent.h"
#include "Define.h"
#include "UObject/ObjectFactory.h"
#include "GizmoCircleComponent.h"
#include "Components/Player.h"
#include "GizmoRectangleComponent.h"
#include "World.h"

UTransformGizmo::UTransformGizmo()
{
	UObject* obj = FObjectFactory::ConstructObject<UGizmoArrowComponent>();
	UGizmoArrowComponent* ArrowX = static_cast<UGizmoArrowComponent*>(obj);
	ArrowX->SetType("ArrowX");
	ArrowX->SetParent(this);
	AttachChildren.Add(ArrowX);
	ArrowArr.Add(ArrowX);

	obj = FObjectFactory::ConstructObject<UGizmoArrowComponent>();
	UGizmoArrowComponent* ArrowY = static_cast<UGizmoArrowComponent*>(obj);
	ArrowY->SetType("ArrowY");

	ArrowY->SetParent(this);
	ArrowY->SetDir(ARROW_DIR::AD_Y);
	AttachChildren.Add(ArrowY);
	ArrowArr.Add(ArrowY);


	obj = FObjectFactory::ConstructObject<UGizmoArrowComponent>();
	UGizmoArrowComponent* ArrowZ = static_cast<UGizmoArrowComponent*>(obj);
	AttachChildren.Add(ArrowZ);
	ArrowZ->SetType("ArrowZ");
	ArrowZ->SetParent(this);
	ArrowZ->SetDir(ARROW_DIR::AD_Z);
	AttachChildren.Add(ArrowZ);
	ArrowArr.Add(ArrowZ);

	UGizmoCircleComponent* disc = new UGizmoCircleComponent();
	disc->SetInnerRadius(0.9f);
	disc->SetType("CircleX");
	disc->SetRotation(FVector(0.0f,0.0f,0.0f));
	disc->SetParent(this);
	AttachChildren.Add(disc);
	CircleArr.Add(disc);

	disc = new UGizmoCircleComponent();
	disc->SetInnerRadius(0.9f);
	disc->SetType("CircleY");
	disc->SetParent(this);
	AttachChildren.Add(disc);
	CircleArr.Add(disc);


	disc = new UGizmoCircleComponent();
	disc->SetInnerRadius(0.9f);
	disc->SetType("CircleZ");
	disc->SetParent(this);
	disc->SetRotation(FVector(0.0f,0.0f,0.0f));
	AttachChildren.Add(disc);
	CircleArr.Add(disc);

	for (auto i : CircleArr)
	{
		i->SetScale({ 2.5f,2.5f,2.5f });
	}

	obj = FObjectFactory::ConstructObject<UGizmoRectangleComponent>();
	UGizmoRectangleComponent* ScaleX = static_cast<UGizmoRectangleComponent*>(obj);
	AttachChildren.Add(ScaleX);
	ScaleX->SetType("ScaleX");
	ScaleX->SetParent(this);
	AttachChildren.Add(ScaleX);
	//GetWorld()->GetObjectArr().Add(ScaleX);
	RectangleArr.Add(ScaleX);

	obj = FObjectFactory::ConstructObject<UGizmoRectangleComponent>();
	UGizmoRectangleComponent* ScaleY = static_cast<UGizmoRectangleComponent*>(obj);
	AttachChildren.Add(ScaleY);
	ScaleY->SetType("ScaleY");
	ScaleY->SetParent(this);
	AttachChildren.Add(ScaleY);
	//GetWorld()->GetObjectArr().Add(ScaleY);
	RectangleArr.Add(ScaleY);

	obj = FObjectFactory::ConstructObject<UGizmoRectangleComponent>();
	UGizmoRectangleComponent* ScaleZ = static_cast<UGizmoRectangleComponent*>(obj);
	AttachChildren.Add(ScaleZ);
	ScaleZ->SetType("ScaleZ");
	ScaleZ->SetParent(this);
	AttachChildren.Add(ScaleZ);
	//GetWorld()->GetObjectArr().Add(ScaleZ);
	RectangleArr.Add(ScaleZ);



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
	for (int i = 0;i < 3;i++)
	{
		ArrowArr[i]->Render();
		CircleArr[i]->Render();
		RectangleArr[i]->Render();
	}
}
