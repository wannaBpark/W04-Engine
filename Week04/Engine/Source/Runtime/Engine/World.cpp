#include "Engine/Source/Runtime/Engine/World.h"

#include "Actors/Player.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "UObject/UObjectIterator.h"
#include "Components/PrimitiveComponent.h"
#include "Runtime/GeometryCore/Octree.h"
#include "GeometryCore/KDTree.h"
#include "GeometryCore/BVHNode.h"
#include "Runtime/Engine/Level.h"


void UWorld::Initialize()
{
    // TODO: Load Scene
    CreateBaseObject();
}

void UWorld::CreateBaseObject()
{
	if (EditorPlayer == nullptr)
	{
		EditorPlayer = FObjectFactory::ConstructObject<AEditorPlayer>();;
	}

	if (LocalGizmo == nullptr)
	{
		LocalGizmo = FObjectFactory::ConstructObject<UTransformGizmo>();
	}
	
	// 단일 Level이라 가정
	if (PersistentLevel == nullptr)
	{
		PersistentLevel = new ULevel();
		PersistentLevel->InitializeLevel(this);
	}
}

void UWorld::ReleaseBaseObject()
{
    if (LocalGizmo)
    {
        delete LocalGizmo;
        LocalGizmo = nullptr;
    }

    if (worldGizmo)
    {
        delete worldGizmo;
        worldGizmo = nullptr;
    }

    if (EditorPlayer)
    {
        delete EditorPlayer;
        EditorPlayer = nullptr;
    }

}

void UWorld::Tick(float DeltaTime)
{
    EditorPlayer->Tick(DeltaTime);
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

	PersistentLevel->Tick(DeltaTime);
}

void UWorld::Release()
{
	PersistentLevel->Release();

	for (AActor* Actor : ActorsArray)
	{
		Actor->EndPlay(EEndPlayReason::WorldTransition);
        TArray<UActorComponent*> CopiedComponents = Actor->GetComponents();
	    for (UActorComponent* Component : CopiedComponents)
	    {
	        GUObjectArray.MarkRemoveObject(Component);
	    }
	    GUObjectArray.MarkRemoveObject(Actor);
	}
    ActorsArray.Empty();

	pickingGizmo = nullptr;
	ReleaseBaseObject();

    GUObjectArray.ProcessPendingDestroyObjects();
}

bool UWorld::DestroyActor(AActor* ThisActor)
{
    if (ThisActor->GetWorld() == nullptr)
    {
        return false;
    }

    if (ThisActor->IsActorBeingDestroyed())
    {
        return true;
    }

    // 액터의 Destroyed 호출
    ThisActor->Destroyed();

    if (ThisActor->GetOwner())
    {
        ThisActor->SetOwner(nullptr);
    }

    TArray<UActorComponent*> CopiedComponents = ThisActor->GetComponents();
    for (UActorComponent* Component : CopiedComponents)
    {
        Component->DestroyComponent();
    }

    // Actor 소유하고 있는 Level 에서 제거
	ULevel* Level = ThisActor->GetLevel();
	if (Level)
	{
		Level->ActorsArray.Remove(ThisActor);
	}
    ActorsArray.Remove(ThisActor);

	// PersistentLevel->RemoveActor()

    // 제거 대기열에 추가
    GUObjectArray.MarkRemoveObject(ThisActor);
    return true;
}

void UWorld::SetPickingGizmo(UObject* Object)
{
	pickingGizmo = Cast<USceneComponent>(Object);
}

void UWorld::SetOctreeSystem(const TArray<UPrimitiveComponent*>& Components)
{
    if (OctreeSystem* Octree = this->GetOctreeSystem())
    {
        delete Octree;

        TArray<UPrimitiveComponent*> WorldComps;
        for (const auto& obj : Components)
        {
            WorldComps.Add(obj);
        }
        Octree = new OctreeSystem();
        this->SetOctreeSystem(Octree);
        Octree->Build(WorldComps);
    }
    else
    {
        // 옥트리가 없으면 새로 생성
        Octree = new OctreeSystem();
        this->SetOctreeSystem(Octree); // 월드에 옥트리 시스템 연결
        Octree->Build(Components);
    }
}

void UWorld::SetKDTreeSystem(const TArray<UPrimitiveComponent*>& Components)
{
    if (KDTreeSystem* KDTree = this->GetKDTreeSystem())
    {
        delete KDTree;

        TArray<UPrimitiveComponent*> WorldComps;
        for (const auto& obj : Components)
        {
            WorldComps.Add(obj);
        }
        KDTree = new KDTreeSystem();
        this->SetKDTreeSystem(KDTree);
        KDTree->Build(WorldComps);
    }
    else
    {
        KDTree = new KDTreeSystem();
        this->SetKDTreeSystem(KDTree); // 월드에 옥트리 시스템 연결
        KDTree->Build(Components);
    }
}

void UWorld::SetBVHSystem(TArray<UPrimitiveComponent*>& Components)
{
    if (BVHSystem* BVH = this->GetBVHSystem())
    {
        delete BVH;

        TArray<UPrimitiveComponent*> WorldComps;
        for (const auto& obj : Components)
        {
            WorldComps.Add(obj);
        }
        BVH = new BVHSystem();
        this->SetBVHSystem(BVH);
        BVH->Build(WorldComps);
    }
    else
    {
        BVH = new BVHSystem();
        this->SetBVHSystem(BVH); // 월드에 옥트리 시스템 연결
        BVH->Build(Components);
    }
}
