#include "StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"


AStaticMeshActor::AStaticMeshActor()
{
    StaticMeshComponent = AddComponent<UStaticMeshComponent>();     // 실제 렌더될 static mesh component 붙임
    RootComponent = StaticMeshComponent;
}
