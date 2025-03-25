#include "Engine/Source/Runtime/Engine/World.h"
#include "Engine/Source/Runtime/Engine/Camera/CameraComponent.h"
#include "Components/SphereComp.h"
#include "Components/CubeComp.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
#include "UObject/ObjectFactory.h"
#include "Actors/Player.h"
#include "BaseGizmos/GizmoArrowComponent.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Components/UBillboardComponent.h"
#include "Components/UText.h"
#include "Components/UParticleSubUVComp.h"
#include "Components/LightComponent.h"
#include "UObject/Casts.h"


UWorld::UWorld()
{
}

UWorld::~UWorld()
{
	Release();
}

void UWorld::Initialize()
{
    // TODO: Load Scene
    CreateBaseObject();
}

void UWorld::CreateBaseObject()
{
    if (localPlayer == nullptr) {
        UObject* player = FObjectFactory::ConstructObject<UPlayer>();
        localPlayer = static_cast<UPlayer*>(player);
    }
    if (camera == nullptr) {
        UObject* Camera = FObjectFactory::ConstructObject<UCameraComponent>();
        camera = static_cast<UCameraComponent*>(Camera);
        camera->SetLocation(FVector(8.0f, 8.0f, 8.f));
        camera->SetRotation(FVector(0.0f, 45.0f, -135.0f));
    }

    if (LocalGizmo == nullptr) {
        UObject* pLocalGizmo = FObjectFactory::ConstructObject<UTransformGizmo>();
        LocalGizmo = static_cast<UTransformGizmo*>(pLocalGizmo);
    }
}

void UWorld::ReleaseBaseObject()
{
	delete LocalGizmo;
	delete worldGizmo;
	delete camera;
	delete localPlayer;
	LocalGizmo = nullptr;
	camera = nullptr;
	localPlayer = nullptr;
}

void UWorld::RenderBaseObject()
{
	LocalGizmo->Render();
}

void UWorld::Tick(float DeltaTime)
{
	camera->TickComponent(DeltaTime);
	localPlayer->Tick(DeltaTime);
	LocalGizmo->Tick(DeltaTime);

    // SpawnActor()에 의해 Actor가 생성된 경우, 여기서 BeginPlay 호출
    for (AActor* Actor : PendingBeginPlayActors)
    {
        Actor->BeginPlay();
    }
    PendingBeginPlayActors.Empty();

    // 매 틱마다 Actor->Tick(...) 호출
	for (AActor* Actor : ActorsArray)
	{
	    Actor->Tick(DeltaTime);
	}

    // 제거 대기열에 있는 Actor들 제거
    for (AActor* Actor : PendingDestroyActors)
    {
        const TSet CopiedComponents = Actor->GetComponents();
        for (UActorComponent* Comp : CopiedComponents)
        {
            Comp->DestroyComponent();
            GUObjectArray.Remove(Comp);
            delete Comp;
        }
        GUObjectArray.Remove(Actor);
        delete Actor;
    }
    PendingDestroyActors.Empty();
}

void UWorld::Release()
{

	for (auto iter : GUObjectArray)
	{
		delete iter;
	}
	GUObjectArray.Empty();
    ActorsArray.Empty();
	pickingGizmo = nullptr;
	ReleaseBaseObject();
}

void UWorld::Render()
{
    for (const auto& Actor : ActorsArray)
    {
        Actor->Render();
        if (ShowFlags::GetInstance().currentFlags & EEngineShowFlags::SF_UUIDText)
        {
            Actor->RenderUUID();
        }
    }

}

bool UWorld::DestroyActor(AActor* Actor)
{
    if (Actor->GetWorld() == nullptr)
    {
        return false;
    }

    if (PendingDestroyActors.Contains(Actor))
    {
        return true;
    }

    // 액터의 Destroyed 호출
    Actor->Destroyed();

    // World에서 제거
    ActorsArray.Remove(Actor);

    // 제거 대기열에 추가
    PendingDestroyActors.Add(Actor);
    return true;
}

void UWorld::SetPickingGizmo(UObject* Object)
{
	pickingGizmo = static_cast<USceneComponent*>(_Obj);
}