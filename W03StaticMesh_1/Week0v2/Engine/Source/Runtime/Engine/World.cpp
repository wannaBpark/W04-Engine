#include "Engine/Source/Runtime/Engine/World.h"
#include "Engine/Source/Runtime/Engine/Camera/CameraComponent.h"
#include "Components/SphereComp.h"
#include "Components/CubeComp.h"
#include <DirectXMath.h>
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
#include "UObject/ObjectFactory.h"
#include "Components/Player.h"
#include "BaseGizmos/GizmoArrowComponent.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Components/UBillboardComponent.h"
#include "Components/UText.h"
#include "Components/UParticleSubUVComp.h"
#include "Components/LightComponent.h"
#include "Components/UTextUUID.h"
#include "Components/SkySphereComponent.h"
#include "UObject/Casts.h"
#include "LevelEditor/SLevelEditor.h"
#include "Engine/FLoaderOBJ.h"

UWorld::UWorld()
{
}

UWorld::~UWorld()
{
	Release();
}

void UWorld::Initialize()
{
    SceneData loadData = FSceneMgr::ParseSceneData(FSceneMgr::LoadSceneFromFile(defaultMapName));
    LoadData(loadData);
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

    // FManagerOBJ::CreateStaticMesh("Assets/Dodge/Dodge.obj");
    // UStaticMeshComponent* mySummerCar = FObjectFactory::ConstructObject<UStaticMeshComponent>();
    // UStaticMesh* tesmp = FManagerOBJ::GetStaticMesh(L"Dodge.obj");
    // mySummerCar->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Dodge.obj"));
    //mySummerCar->GetMaterial(0)->SetDiffuse(FVector(0.3f,0.4f,0.5f));
    // mySummerCar->GetMaterial(1)->SetDiffuse(FVector(1.0f,0.2f,0.2f));
    //mySummerCar->GetMaterial(2)->SetDiffuse(FVector(0.0f,0.4f,0.4f));
    //mySummerCar->GetMaterial(3)->SetDiffuse(FVector(0.8f,0.8f,0.0f));
    // GUObjectArray.Add(mySummerCar);
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

void UWorld::Tick(double deltaTime)
{
	Input();
	camera->Update(deltaTime);
	localPlayer->Update(deltaTime);
	LocalGizmo->Update(deltaTime);
	for (auto iter : GUObjectArray)
	{
		iter->Update(deltaTime);
	}
	
}

void UWorld::Release()
{

	for (auto iter : GUObjectArray)
	{
		delete iter;
	}
	GUObjectArray.Empty();
	pickingObj = nullptr;
	pickingGizmo = nullptr;
	ReleaseBaseObject();
}

void UWorld::Render()
{
	for (auto iter : GUObjectArray)
	{
		iter->Render();
		if ((GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_UUIDText))) 
			iter->RenderUUID();
	}

}

void UWorld::Input()
{

}

void UWorld::SpawnObject(OBJECTS _Obj)
{
	UObject* pObj = nullptr;
	switch (_Obj)
	{
	case OBJ_SPHERE:
		pObj = FObjectFactory::ConstructObject<USphereComp>();
		GUObjectArray.Add(pObj);
		break;
	case OBJ_TRIANGLE:
		break;
	case OBJ_CUBE:
		pObj = FObjectFactory::ConstructObject<UCubeComp>();
		GUObjectArray.Add(pObj);
		break;
	case OBJ_SpotLight:
	{
		UObject* spotLight = FObjectFactory::ConstructObject<ULightComponentBase>();
		spotLight = static_cast<ULightComponentBase*>(spotLight);
		ULightComponentBase* castLight = static_cast<ULightComponentBase*>(spotLight);

		GUObjectArray.Add(spotLight);
		break;
	}
	case OBJ_PARTICLE:
	{
		UObject* particle = FObjectFactory::ConstructObject<UParticleSubUVComp>();
		UParticleSubUVComp* castParticle = static_cast<UParticleSubUVComp*>(particle);
		castParticle->SetTexture(L"Assets/Texture/T_Explosion_SubUV.png");
		castParticle->SetRowColumnCount(6, 6);
		castParticle->SetScale(FVector(10.0f, 10.0f, 1.0f));
		GUObjectArray.Add(castParticle);
	}
	break;
	case OBJ_Text:
	{
		UObject* text = FObjectFactory::ConstructObject<UText>();
		UText* castText = static_cast<UText*>(text);
		castText->SetTexture(L"Assets/Texture/font.png");
		castText->SetRowColumnCount(106, 106);
		castText->SetText(L"안녕하세요 Jungle 1");
		//SetText전에 RowColumn 반드시 설정
		GUObjectArray.Add(text);
	}
	break;
	default:
		break;
	}
	pickingObj = static_cast<USceneComponent*>(pObj);
}

void UWorld::LoadData(SceneData& _Data)
{
	Release();
	for (auto iter : _Data.Primitives)
	{
		GUObjectArray.Add(iter.Value);
	}
    if(_Data.Cameras[0])
        camera = static_cast<UCameraComponent*>(_Data.Cameras[0]);
	CreateBaseObject();
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

void UWorld::SetPickingObj(UObject* _Obj)
{
	pickingObj = static_cast<USceneComponent*>(_Obj);
    OnSelectedObject.ExecuteIfBound(pickingObj);
}

void UWorld::DeleteObj(UObject* _Obj)
{
	UObject* tmpObj = _Obj;
    Trashbin.Remove(tmpObj);
	delete tmpObj;
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