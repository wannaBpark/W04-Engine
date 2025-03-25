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
    FManagerOBJ::CreateStaticMesh("Assets/LocationGizmo.obj");
    FManagerOBJ::CreateStaticMesh("Assets/RotationGizmo.obj");
    FManagerOBJ::CreateStaticMesh("Assets/ScaleGizmo.obj");

    FObjMaterialInfo YAxisMaterialInfo = {
        "YAxisMaterial",                // MTLName
        false,                         // bHasTexture
        false,                         // bTransparent
        FVector(0.0f, 1.0f, 0.0f),     // Diffuse
        FVector(1.0f, 1.0f, 1.0f),     // Specular
        FVector(0.2f, 0.2f, 0.2f),     // Ambient
        FVector(0.0f, 0.0f, 0.0f),     // Emissive
        32.0f,                         // SpecularScalar
        1.0f,                          // DensityScalar
        1.0f,                          // TransparencyScalar
    };
    FObjMaterialInfo ZAxisMaterialInfo = {
    "ZAxisMaterial",                // MTLName
    false,                         // bHasTexture
    false,                         // bTransparent
    FVector(0.0f, 0.0f, 1.0f),     // Diffuse
    FVector(1.0f, 1.0f, 1.0f),     // Specular
    FVector(0.2f, 0.2f, 0.2f),     // Ambient
    FVector(0.0f, 0.0f, 0.0f),     // Emissive
    32.0f,                         // SpecularScalar
    1.0f,                          // DensityScalar
    1.0f,                          // TransparencyScalar
    };
    FManagerOBJ::CreateMaterial(YAxisMaterialInfo);
    FManagerOBJ::CreateMaterial(ZAxisMaterialInfo);

    UStaticMeshComponent* locationX = FObjectFactory::ConstructObject<UStaticMeshComponent>();
    locationX->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"LocationGizmo.obj"));
    locationX->SetRotation(FVector(0, 0, 90));
	locationX->SetParent(this);
	AttachChildren.Add(locationX);
	ArrowArr.Add(locationX);

    UStaticMeshComponent* locationY = FObjectFactory::ConstructObject<UStaticMeshComponent>();
    UMaterial* YMaterial = FManagerOBJ::GetMaterial("YAxisMaterial");
    locationY->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"LocationGizmo.obj"));
    locationY->SetRotation(FVector(0, 0, 180));
    locationY->SetMaterial(0, YMaterial);
    locationY->SetParent(this);
    AttachChildren.Add(locationY);
    ArrowArr.Add(locationY);

    UStaticMeshComponent* locationZ = FObjectFactory::ConstructObject<UStaticMeshComponent>();
    UMaterial* ZMaterial = FManagerOBJ::GetMaterial("ZAxisMaterial");
    locationZ->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"LocationGizmo.obj"));
    locationZ->SetRotation(FVector(-90, 0, 0));
    locationZ->SetMaterial(0, ZMaterial);
    locationZ->SetParent(this);
    AttachChildren.Add(locationZ);
    ArrowArr.Add(locationZ);

	//ArrowY->SetDir(ARROW_DIR::AD_Y);
	//ArrowZ->SetDir(ARROW_DIR::AD_Z);

	//UGizmoCircleComponent* disc = new UGizmoCircleComponent();
	//disc->SetInnerRadius(0.9f);
	//disc->SetType("CircleX");
	//disc->SetRotation(FVector(0.0f,0.0f,0.0f));
	//disc->SetParent(this);
	//AttachChildren.Add(disc);
	//CircleArr.Add(disc);

	//disc = new UGizmoCircleComponent();
	//disc->SetInnerRadius(0.9f);
	//disc->SetType("CircleY");
	//disc->SetParent(this);
	//AttachChildren.Add(disc);
	//CircleArr.Add(disc);


	//disc = new UGizmoCircleComponent();
	//disc->SetInnerRadius(0.9f);
	//disc->SetType("CircleZ");
	//disc->SetParent(this);
	//disc->SetRotation(FVector(0.0f,0.0f,0.0f));
	//AttachChildren.Add(disc);
	//CircleArr.Add(disc);

	//for (auto i : CircleArr)
	//{
	//	i->SetScale({ 2.5f,2.5f,2.5f });
	//}

	//obj = FObjectFactory::ConstructObject<UGizmoRectangleComponent>();
	//UGizmoRectangleComponent* ScaleX = static_cast<UGizmoRectangleComponent*>(obj);
	//AttachChildren.Add(ScaleX);
	//ScaleX->SetType("ScaleX");
	//ScaleX->SetParent(this);
	//AttachChildren.Add(ScaleX);
	////GetWorld()->GetObjectArr().Add(ScaleX);
	//RectangleArr.Add(ScaleX);

	//obj = FObjectFactory::ConstructObject<UGizmoRectangleComponent>();
	//UGizmoRectangleComponent* ScaleY = static_cast<UGizmoRectangleComponent*>(obj);
	//AttachChildren.Add(ScaleY);
	//ScaleY->SetType("ScaleY");
	//ScaleY->SetParent(this);
	//AttachChildren.Add(ScaleY);
	////GetWorld()->GetObjectArr().Add(ScaleY);
	//RectangleArr.Add(ScaleY);

	//obj = FObjectFactory::ConstructObject<UGizmoRectangleComponent>();
	//UGizmoRectangleComponent* ScaleZ = static_cast<UGizmoRectangleComponent*>(obj);
	//AttachChildren.Add(ScaleZ);
	//ScaleZ->SetType("ScaleZ");
	//ScaleZ->SetParent(this);
	//AttachChildren.Add(ScaleZ);
	////GetWorld()->GetObjectArr().Add(ScaleZ);
	//RectangleArr.Add(ScaleZ);
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
		//CircleArr[i]->Update(deltaTime);
		//RectangleArr[i]->Update(deltaTime);
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
		//CircleArr[i]->Render();
		//RectangleArr[i]->Render();
	}
}
