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
	////테스트용 텍스트
	
	/*UObject* pObj = FObjectFactory::ConstructObject<USkySphereComponent>();
	USkySphereComponent* skySphere = static_cast<USkySphereComponent*>(pObj);
	skySphere->SetTexture(L"Assets/Texture/ocean_sky.jpg");
	skySphere->SetScale(FVector( -300.0f, -300.0f, -300.0f));
	skySphere->SetRotation(FVector(-167.0f, 25.0f, -135.0f));

	GUObjectArray.Add(skySphere);*/


	//테스트용 텍스트
	
	
/*

	//테스트용 빌보드. 필요없으면 삭제
	UObject* billboard = FObjectFactory::ConstructObject<UBillboardComponent>();
	billboard = static_cast<UBillboardComponent*>(billboard);
	UBillboardComponent* castBillboard = static_cast<UBillboardComponent*>(billboard);
	castBillboard->SetTexture(L"Assets/Texture/emart.png");
	GUObjectArray.Add(billboard);


	/*
	//테스트용 텍스트
	UObject* uuid = FObjectFactory::ConstructObject<UTextUUID>();
	UTextUUID* castUUID = static_cast<UTextUUID*>(uuid);
	castUUID->SetTexture(L"Assets/Texture/font.png");
	castUUID->SetRowColumnCount(106, 106);
	castUUID->SetUUID(sphere->UUID);
	castUUID->SetScale(FVector(0.25f, 0.25f, 0.25f));
	//SetText전에 RowColumn 반드시 설정
	GUObjectArray.Add(uuid);

	castUUID->SetUUIDParent(sphere);
	//castBillboard->SetupAttachment(sphere);
	//sphere->AddChild(castBillboard);
	//cube->AddChild(sphere);
*/
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
	Input();
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

void UWorld::Input()
{

}

void UWorld::LoadData(SceneData& _Data)
{
	Release();
	CreateBaseObject();
	for (auto iter : _Data.Primitives)
	{
		GUObjectArray.Add(iter.Value);
	}

    camera = static_cast<UCameraComponent*>(_Data.Cameras[0]);
}

SceneData UWorld::SaveData()
{
	SceneData Save;
	int32 Count = 0;
	for (UObject* iter : GUObjectArray)
	{
	    if (const USceneComponent* Primitive = Cast<USceneComponent>(iter))
	    {
            if (!Primitive->IsA<UBillboardComponent>())
            {
                Save.Primitives[Count] = iter;
                Count++;
            }
		}
	}
    // TODO :
    // 이후 카메라가 여러 대로 바뀌면 루프로 바꾸기
    Save.Cameras[0] = GetCamera();
	Save.Version = 1;
	Save.NextUUID = Count;
	
	return Save;
}

void UWorld::NewScene()
{
	Release();
	CreateBaseObject();
}

void UWorld::ThrowAwayObj(UObject* _Obj)
{
	Trashbin.Add(_Obj);
}

void UWorld::CleanUp()
{
	if (Trashbin.IsEmpty())
		return;

    for (const auto& Obj : Trashbin)
    {
        GUObjectArray.Remove(Obj);
        delete Obj;
    }
    Trashbin.Empty();
}

void UWorld::SetPickingGizmo(UObject* _Obj)
{
	pickingGizmo = static_cast<USceneComponent*>(_Obj);
}