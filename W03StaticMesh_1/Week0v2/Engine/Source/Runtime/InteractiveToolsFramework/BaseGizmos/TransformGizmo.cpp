#include "TransformGizmo.h"
#include "GizmoArrowComponent.h"
#include "Define.h"
#include "GizmoCircleComponent.h"
#include "Actors/Player.h"
#include "GizmoRectangleComponent.h"
#include "World.h"

UTransformGizmo::UTransformGizmo()
{
    SetRootComponent(
        AddComponent<USceneComponent>()
    );

	UGizmoArrowComponent* ArrowX = AddComponent<UGizmoArrowComponent>();
	ArrowX->SetupAttachment(RootComponent);
	ArrowX->SetType("ArrowX");
	ArrowArr.Add(ArrowX);

	UGizmoArrowComponent* ArrowY = AddComponent<UGizmoArrowComponent>();
	ArrowY->SetupAttachment(RootComponent);
	ArrowY->SetType("ArrowY");
	ArrowY->SetDir(ARROW_DIR::AD_Y);
	ArrowArr.Add(ArrowY);


	UGizmoArrowComponent* ArrowZ = AddComponent<UGizmoArrowComponent>();
	ArrowZ->SetupAttachment(RootComponent);
	ArrowZ->SetType("ArrowZ");
	ArrowZ->SetDir(ARROW_DIR::AD_Z);
	ArrowArr.Add(ArrowZ);

	UGizmoCircleComponent* disc = AddComponent<UGizmoCircleComponent>();
	disc->SetupAttachment(RootComponent);
	disc->SetInnerRadius(0.9f);
	disc->SetType("CircleX");
	disc->SetRotation(FVector(0.0f,0.0f,0.0f));
	CircleArr.Add(disc);

	disc = AddComponent<UGizmoCircleComponent>();
	disc->SetupAttachment(RootComponent);
	disc->SetInnerRadius(0.9f);
	disc->SetType("CircleY");
	CircleArr.Add(disc);


	disc = AddComponent<UGizmoCircleComponent>();
	disc->SetupAttachment(RootComponent);
	disc->SetInnerRadius(0.9f);
	disc->SetType("CircleZ");
	disc->SetRotation(FVector(0.0f,0.0f,0.0f));
	CircleArr.Add(disc);

	for (auto i : CircleArr)
	{
		i->SetScale({ 2.5f,2.5f,2.5f });
	}

	UGizmoRectangleComponent* ScaleX = AddComponent<UGizmoRectangleComponent>();
	ScaleX->SetType("ScaleX");
	ScaleX->SetupAttachment(RootComponent);
	RectangleArr.Add(ScaleX);

	UGizmoRectangleComponent* ScaleY = AddComponent<UGizmoRectangleComponent>();
	ScaleY->SetType("ScaleY");
	ScaleY->SetupAttachment(RootComponent);
	RectangleArr.Add(ScaleY);

	UGizmoRectangleComponent* ScaleZ = AddComponent<UGizmoRectangleComponent>();
	ScaleZ->SetType("ScaleZ");
	ScaleZ->SetupAttachment(RootComponent);
	RectangleArr.Add(ScaleZ);



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
		CircleArr[i]->TickComponent(DeltaTime);
		RectangleArr[i]->TickComponent(DeltaTime);
	}
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
