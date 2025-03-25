#include "CubeComp.h"
#include "Math/JungleMath.h"
#include "World.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "UObject/ObjectFactory.h"
#include "PropertyEditor/ShowFlags.h"

#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"
UCubeComp::UCubeComp()
{
    SetType(StaticClass()->GetName());
    AABB.max = { 1,1,1 };
    AABB.min = { -1,-1,-1 };
}

UCubeComp::~UCubeComp()
{
}

void UCubeComp::InitializeComponent()
{
    Super::InitializeComponent();
}

void UCubeComp::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

}