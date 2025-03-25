#include "TransformGizmo.h"
#include "GizmoArrowComponent.h"
#include "Define.h"
#include "GizmoCircleComponent.h"
#include "Actors/Player.h"
#include "GizmoRectangleComponent.h"
#include "World.h"
#include "Engine/FLoaderOBJ.h"

UTransformGizmo::UTransformGizmo()
{
    FManagerOBJ::CreateStaticMesh("Assets/LocationGizmo.obj");
    FManagerOBJ::CreateStaticMesh("Assets/RotationGizmo.obj");
    FManagerOBJ::CreateStaticMesh("Assets/ScaleGizmo.obj");

    FObjMaterialInfo YAxisMaterialInfo = {
        .MTLName = "YAxisMaterial",            // MTLName
        .bHasTexture = false,                  // bHasTexture
        .bTransparent = false,                 // bTransparent
        .Diffuse = FVector(0.0f, 1.0f, 0.0f),  // Diffuse
        .Specular = FVector(1.0f, 1.0f, 1.0f), // Specular
        .Ambient = FVector(0.2f, 0.2f, 0.2f),  // Ambient
        .Emissive = FVector(0.0f, 0.0f, 0.0f), // Emissive
        .SpecularScalar = 32.0f,               // SpecularScalar
        .DensityScalar = 1.0f,                 // DensityScalar
        .TransparencyScalar = 1.0f,            // TransparencyScalar
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

    SetRootComponent(
        AddComponent<USceneComponent>()
    );

    UStaticMeshComponent* LocationX = AddComponent<UStaticMeshComponent>();
    LocationX->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"LocationGizmo.obj"));
    LocationX->SetRotation(FVector(0, 0, 90));
    LocationX->SetupAttachment(RootComponent);
    ArrowArr.Add(LocationX);

    UStaticMeshComponent* LocationY = AddComponent<UStaticMeshComponent>();
    UMaterial* YMaterial = FManagerOBJ::GetMaterial("YAxisMaterial");
    LocationY->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"LocationGizmo.obj"));
    LocationY->SetRotation(FVector(0, 0, 180));
    LocationY->SetMaterial(0, YMaterial);
    LocationY->SetupAttachment(RootComponent);
    ArrowArr.Add(LocationY);

    UStaticMeshComponent* LocationZ = AddComponent<UStaticMeshComponent>();
    UMaterial* ZMaterial = FManagerOBJ::GetMaterial("ZAxisMaterial");
    LocationZ->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"LocationGizmo.obj"));
    LocationZ->SetRotation(FVector(-90, 0, 0));
    LocationZ->SetMaterial(0, ZMaterial);
    LocationZ->SetupAttachment(RootComponent);
    ArrowArr.Add(LocationZ);
}

void UTransformGizmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (const AActor* PickedActor = GetWorld()->GetPickedActor())
    {
        SetActorLocation(PickedActor->GetActorLocation());
        if (GetWorld()->GetPlayer()->GetCoordiMode() == CoordiMode::CDM_LOCAL)
        {
            // TODO: 임시로 RootComponent의 정보로 사용
            SetActorRotation(PickedActor->GetActorRotation());
        }
        else if (GetWorld()->GetPlayer()->GetCoordiMode() == CoordiMode::CDM_WORLD)
            SetActorRotation(FVector(0.0f, 0.0f, 0.0f));
    }
	for (int i = 0;i < 3;i++)
	{
		ArrowArr[i]->TickComponent(DeltaTime);
		//CircleArr[i]->TickComponent(DeltaTime);
		//RectangleArr[i]->TickComponent(DeltaTime);
	}
}