#include "Engine/Source/Runtime/Engine/World.h"

#include "Actors/Player.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GeometryCore/BVHNode.h"
#include "GeometryCore/KDTree.h"
#include "LevelEditor/SLevelEditor.h"
#include "Runtime/Engine/Level.h"
#include "Runtime/GeometryCore/Octree.h"
#include "UObject/UObjectArray.h"
#include "UObject/UObjectIterator.h"


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
		LocalGizmo = FObjectFactory::ConstructObject<ATransformGizmo>();
	}
	
	// 단일 Level이라 가정
	if (PersistentLevel == nullptr)
	{
		PersistentLevel = FObjectFactory::ConstructObject<ULevel>();
		PersistentLevel->InitializeLevel(this);
	}
}

void UWorld::ReleaseBaseObject()
{
    if (LocalGizmo)
    {
        LocalGizmo->MarkAsGarbage();
        LocalGizmo = nullptr;
    }

    if (EditorPlayer)
    {
        EditorPlayer->MarkAsGarbage();
        EditorPlayer = nullptr;
    }
}

void UWorld::Tick(float DeltaTime)
{
    EditorPlayer->Tick(DeltaTime);
	LocalGizmo->Tick(DeltaTime);
	PersistentLevel->Tick(DeltaTime);
}

void UWorld::Release()
{
	PersistentLevel->Release();
	GUObjectArray.MarkRemoveObject(PersistentLevel);
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
