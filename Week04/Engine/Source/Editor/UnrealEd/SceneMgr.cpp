#include "UnrealEd/SceneMgr.h"
#include <fstream>
#include <tinyfiledialogs/tinyfiledialogs.h>
#include "BaseGizmos/GizmoArrowComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CubeComp.h"
#include "Components/LightComponent.h"
#include "Components/SkySphereComponent.h"
#include "Components/SphereComp.h"
#include "Components/UBillboardComponent.h"
#include "Editor/LevelEditor/SLevelEditor.h"
#include "Editor/UnrealEd/EditorViewportClient.h"
#include "Engine/FLoaderOBJ.h"
#include "Engine/StaticMeshActor.h"
#include "JSON/json.hpp"
#include "Runtime/Engine/World.h"
#include "UObject/Casts.h"
#include "UObject/Object.h"
#include "UObject/ObjectFactory.h"
using json = nlohmann::json;

//JSON 문자열 파싱해서 Scene Data 객체 구성.
SceneData FSceneMgr::ParseSceneData(const FString& jsonStr)
{
    SceneData sceneData;

    try {
        json j = json::parse(*jsonStr);

        // 버전과 NextUUID 읽기
        sceneData.Version = j["Version"].get<int>();
        sceneData.NextUUID = j["NextUUID"].get<int>();

        // Primitives 처리 (C++14 스타일)
        auto primitives = j["Primitives"];
        for (auto it = primitives.begin(); it != primitives.end(); ++it) {
            int id = std::stoi(it.key());  // Key는 문자열, 숫자로 변환
            const json& value = it.value();
            UObject* obj = nullptr;
            if (value.contains("Type"))
            {
                const FString TypeName = value["Type"].get<std::string>();
                if (TypeName == USphereComp::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<USphereComp>();
                }
                else if (TypeName == UCubeComp::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<UCubeComp>();
                }
                else if (TypeName == UGizmoArrowComponent::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<UGizmoArrowComponent>();
                }
                else if (TypeName == UBillboardComponent::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<UBillboardComponent>();
                }
                else if (TypeName == ULightComponentBase::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<ULightComponentBase>();
                }
                else if (TypeName == USkySphereComponent::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<USkySphereComponent>();
                    USkySphereComponent* skySphere = static_cast<USkySphereComponent*>(obj);
                }
            }

            USceneComponent* sceneComp = static_cast<USceneComponent*>(obj);
            //Todo : 여기다가 Obj Maeh저장후 일기
            //if (value.contains("ObjStaticMeshAsset"))
            if (value.contains("Location")) sceneComp->SetLocation(FVector(value["Location"].get<std::vector<float>>()[0],
                value["Location"].get<std::vector<float>>()[1],
                value["Location"].get<std::vector<float>>()[2]));
            if (value.contains("Rotation")) sceneComp->SetRotation(FVector(value["Rotation"].get<std::vector<float>>()[0],
                value["Rotation"].get<std::vector<float>>()[1],
                value["Rotation"].get<std::vector<float>>()[2]));
            if (value.contains("Scale")) sceneComp->SetScale(FVector(value["Scale"].get<std::vector<float>>()[0],
                value["Scale"].get<std::vector<float>>()[1],
                value["Scale"].get<std::vector<float>>()[2]));
            if (value.contains("Type")) {
                UPrimitiveComponent* primitiveComp = Cast<UPrimitiveComponent>(sceneComp);
                if (primitiveComp) {
                    primitiveComp->SetType(value["Type"].get<std::string>());
                }
                else {
                    std::string name = value["Type"].get<std::string>();
                    sceneComp->NamePrivate = name.c_str();
                }
            }
            sceneData.Primitives[id] = sceneComp;
        }

        auto perspectiveCamera = j["PerspectiveCamera"];
        for (auto it = perspectiveCamera.begin(); it != perspectiveCamera.end(); ++it) {
            int id = std::stoi(it.key());  // Key는 문자열, 숫자로 변환
            const json& value = it.value();
            UObject* obj = FObjectFactory::ConstructObject<UCameraComponent>();
            UCameraComponent* camera = static_cast<UCameraComponent*>(obj);
            if (value.contains("Location")) camera->SetLocation(FVector(value["Location"].get<std::vector<float>>()[0],
                    value["Location"].get<std::vector<float>>()[1],
                    value["Location"].get<std::vector<float>>()[2]));
            if (value.contains("Rotation")) camera->SetRotation(FVector(value["Rotation"].get<std::vector<float>>()[0],
                value["Rotation"].get<std::vector<float>>()[1],
                value["Rotation"].get<std::vector<float>>()[2]));
            if (value.contains("Rotation")) camera->SetRotation(FVector(value["Rotation"].get<std::vector<float>>()[0],
                value["Rotation"].get<std::vector<float>>()[1],
                value["Rotation"].get<std::vector<float>>()[2]));
            if (value.contains("FOV")) camera->SetFOV(value["FOV"].get<float>());
            if (value.contains("NearClip")) camera->SetNearClip(value["NearClip"].get<float>());
            if (value.contains("FarClip")) camera->SetNearClip(value["FarClip"].get<float>());
           
        }
    }
    catch (const std::exception& e) {
        FString errorMessage = "Error parsing JSON: ";
        errorMessage += e.what();

        UE_LOG(LogLevel::Error, "No Json file");
    }

    return sceneData;
}

bool FSceneMgr::LoadSceneFromFile(const FString& filename)
{
    if (filename.IsEmpty()) {
        tinyfd_messageBox("Error", "파일을 불러올 수 없습니다.", "ok", "error", 1);
        ImGui::End();
        return false;
    }

    std::ifstream sceneFile(*filename);
    if (!sceneFile.is_open()) {
        tinyfd_messageBox("Error", "파일을 열 수 없습니다.", "ok", "error", 1);
        ImGui::End();
        return false;
    }

    std::stringstream buffer;
    buffer << sceneFile.rdbuf();

    json jsonData = json::parse(buffer.str());
    UWorld* World = GEngineLoop.GetWorld();

    // TODO : Parsing 부분 함수 분리 필요.
    try {
#pragma region 멀티 뷰포트 관련 로드
        //if (jsonData.contains("PerspectiveCamera"))
        //{
        //    auto camData = jsonData["PerspectiveCamera"];

        //    std::shared_ptr<FEditorViewportClient>* clients = GEngineLoop.GetLevelEditor()->GetViewports();

        //    if (clients)
        //    {
        //        for (auto& [key, value] : camData.items())
        //        {
        //            int index = std::stoi(key);
        //            if (index < 0 || index >= 4) continue;
        //            if (clients[index] == nullptr) continue;

        //            auto& client = clients[index];

        //            // 위치 및 회전
        //            FVector Location, Rotation;
        //            if (value.contains("Location") && value["Location"].is_array())
        //                Location = FVector(value["Location"][0], value["Location"][1], value["Location"][2]);

        //            if (value.contains("Rotation") && value["Rotation"].is_array())
        //                Rotation = FVector(value["Rotation"][0], value["Rotation"][1], value["Rotation"][2]);

        //            float FOV = value.contains("FOV") ? value["FOV"].get<float>() : 60.0f;
        //            float nearClip = value.contains("NearClip") ? value["NearClip"].get<float>() : 0.1f;
        //            float farClip = value.contains("FarClip") ? value["FarClip"].get<float>() : 100000.0f;

        //            client->ViewTransformPerspective.SetLocation(Location);
        //            client->ViewTransformPerspective.SetRotation(Rotation);
        //            client->ViewFOV = FOV;
        //            client->nearPlane = nearClip;
        //            client->farPlane = farClip;

        //            client->UpdateViewMatrix();
        //            client->UpdateProjectionMatrix();
        //        }
        //    }
        //}
#pragma endregion
#pragma region 단일 뷰포트 관련 로드
        if (jsonData.contains("PerspectiveCamera"))
        {
            auto camData = jsonData["PerspectiveCamera"];
            FVector Location(camData["Location"][0], camData["Location"][1], camData["Location"][2]);
            FVector Rotation(camData["Rotation"][0], camData["Rotation"][1], camData["Rotation"][2]);
            float FOV = camData["FOV"][0];
            float nearClip = camData["NearClip"][0];
            float farClip = camData["FarClip"][0];
            
            std::shared_ptr<FEditorViewportClient>* clients = GEngineLoop.GetLevelEditor()->GetViewports();

            // 모든 뷰포트에 set하지만 실질적으로 perspective viewport에만 제대로 적용.
            for (int i = 0; i < 4; i++) {
                auto& client = clients[i];
                client->ViewTransformPerspective.SetLocation(Location);
                client->ViewTransformPerspective.SetRotation(Rotation);
                client->ViewFOV = FOV;
                client->nearPlane = nearClip;
                client->farPlane = farClip;
                client->UpdateViewMatrix();
                client->UpdateProjectionMatrix();
            }
        }
#pragma endrigon
        // [NOTE] UStaticMeshComp 배열을 옥트리 시스템에 추가 필요
        TArray<UPrimitiveComponent*> StaticComps;
        if (jsonData.contains("Primitives"))
        {
            auto& primitives = jsonData["Primitives"];
            for (auto& [uuid, obj] : primitives.items())
            {
                std::string type = obj["Type"];
                FVector location(obj["Location"][0], obj["Location"][1], obj["Location"][2]);
                FVector rotation(obj["Rotation"][0], obj["Rotation"][1], obj["Rotation"][2]);
                FVector scale(obj["Scale"][0], obj["Scale"][1], obj["Scale"][2]);

                if (type == "StaticMeshComp")
                {
                    std::string objPath = std::string(obj["ObjStaticMeshAsset"]);
                    FManagerOBJ::CreateStaticMesh(objPath);

                    AStaticMeshActor* actor = World->SpawnActor<AStaticMeshActor>();
                    std::filesystem::path filePath(objPath);
                    std::string fileNameOnly = filePath.filename().string(); // 파일명만 추출

                    actor->SetActorLabel(TEXT("Loaded_StaticMesh"));
                    UStaticMeshComponent* MeshComp = actor->GetStaticMeshComponent();
                    MeshComp->SetStaticMesh(FManagerOBJ::GetStaticMesh(FString(fileNameOnly).ToWideString()));
                    actor->SetActorLocation(location);
                    actor->SetActorRotation(rotation);
                    actor->SetActorScale(scale);

                    World->SetPickedActor(actor);
                    StaticComps.Add(MeshComp);
                }

            }
        }
        World->SetOctreeSystem(StaticComps);
        World->SetKDTreeSystem(StaticComps);
    }
    catch (const std::exception& e) {
        UE_LOG(LogLevel::Error, "Error parsing JSON: %s", e.what());
        return false;
    }

    return true;
   // inFile.close();

    //return j.dump(4);
}

// SceneData 객체를 json 문자열로 직렬화.
std::string FSceneMgr::SerializeSceneData(const SceneData& sceneData)
{
    json j;

    // Version과 NextUUID 저장
    // TODO : Version정보 제거
    //j["Version"] = sceneData.Version;
    j["NextUUID"] = sceneData.NextUUID;
    
#pragma region primitiveComponent 관련 저장 주석
    // FIXME : worldData bake 과정 수정 후 확인 필요.
    // Primitives 처리 (C++17 스타일)
   /* for (const auto& [Id, Obj] : sceneData.Primitives)
    {
        USceneComponent* primitive = static_cast<USceneComponent*>(Obj);
        std::vector<float> Location = { primitive->GetWorldLocation().x,primitive->GetWorldLocation().y,primitive->GetWorldLocation().z };
        std::vector<float> Rotation = { primitive->GetWorldRotation().x,primitive->GetWorldRotation().y,primitive->GetWorldRotation().z };
        std::vector<float> Scale = { primitive->GetWorldScale().x,primitive->GetWorldScale().y,primitive->GetWorldScale().z };

        std::string primitiveName = *primitive->GetName();
        size_t pos = primitiveName.rfind('_');
        if (pos != INDEX_NONE) {
            primitiveName = primitiveName.substr(0, pos);
        }
        json primitiveJson = {
             {"Location", Location},
             {"Rotation", Rotation},
             {"Scale", Scale},
             {"Type", primitiveName}
        };

        */
        // //StaticMesh라면 추가 정보
        //if (primitiveName == "StaticMeshComp")
        //{
        //    UStaticMeshComponent* staticMesh = dynamic_cast<UStaticMeshComponent*>(primitive);
        //    if (staticMesh && staticMesh->GetStaticMesh())
        //    {
        //        FWString name = staticMesh->GetStaticMesh()->GetRenderData()->ObjectName;
        //        //std::string str(name.begin(), name.end());
        //        j["ObjStaticMeshAsset"] = name; // 예: "Data/apple_mid.obj"
        //    }
        //}

   //       j["Primitives"][std::to_string(Id)] = primitiveJson;
 //}
#pragma endregion

#pragma region 멀티 뷰포트 관련 씬 저장 주석

    //std::shared_ptr<FEditorViewportClient>* viewportClient = GEngineLoop.GetLevelEditor()->GetViewports();
    //if (viewportClient) {
    //    // FIXME : 카메라 id값 임의 설정
    //    int id = 0;
    //    for (auto& client : std::span(viewportClient, 4))
    //    {
    //        if(!client) continue;
    //        const FVector& loc = client->ViewTransformPerspective.GetLocation();
    //        const FVector& rot = client->ViewTransformPerspective.GetRotation();
    //        float fov = client->ViewFOV;
    //        float nearClip = client->nearPlane;
    //        float farClip = client->farPlane;

    //        j["PerspectiveCamera"][std::to_string(id)] = {
    //            {"Location", { loc.x, loc.y, loc.z }},
    //            {"Rotation", { rot.x, rot.y, rot.z }},
    //            {"FOV", fov},
    //            {"NearClip", nearClip},
    //            {"FarClip", farClip}
    //        };
    //        id++; 
    //    }
    //}
#pragma endregion

#pragma region 싱글 뷰포트 관련 씬 저장
    // 활성화 뷰포트 기준 저장.
    std::shared_ptr<FEditorViewportClient> client = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();
    
    //FIXME : 예외 처리
    if (!client)    return "";
    const FVector& loc = client->ViewTransformPerspective.GetLocation();
    const FVector& rot = client->ViewTransformPerspective.GetRotation();
    float fov = client->ViewFOV;
    float nearClip = client->nearPlane;
    float farClip = client->farPlane;

    j["PerspectiveCamera"] = {
        {"Location", { loc.x, loc.y, loc.z }},
        {"Rotation", { rot.x, rot.y, rot.z }},
        {"FOV",       std::vector<float>{fov}},
        {"NearClip",  std::vector<float>{nearClip}},   
        {"FarClip",   std::vector<float>{farClip}}     
    };
#pragma endregion
    return j.dump(4); // 4는 들여쓰기 수준
}

// SceneData를 json으로 직렬화하여 파일에 저장.
bool FSceneMgr::SaveSceneToFile(const FString& filename)
{
    SceneData sceneData = GetWorldSceneData();
    std::ofstream outFile(*filename);
    if (!outFile) {
        FString errorMessage = "Failed to open file for writing: ";
        MessageBoxA(NULL, *errorMessage, "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    std::string jsonData = SerializeSceneData(sceneData);
    outFile << jsonData;
    outFile.close();

    return true;
}

//FIXME : 현재 primitive component 들 데이터 bake 안되는 상태.
SceneData FSceneMgr::GetWorldSceneData() {
    SceneData sceneData;
    sceneData.Version = 1;
    sceneData.NextUUID = 0;

    const TSet<AActor*>& Actors = GEngineLoop.GetWorld()->GetActors();

    for (AActor* Actor : Actors)
    {
        if (!Actor)
            continue;

        // 우선 루트 컴포넌트를 Primitive로 저장
        if (USceneComponent* RootComp = Actor->GetRootComponent())
        {
            int32 Id = sceneData.NextUUID++;
            sceneData.Primitives[Id] = RootComp;
        }

        // Actor 내부에 CameraComponent가 있을 경우 찾아 저장
        // 여기선 RootComp 외에 camera 같은 멤버를 직접 가정
        if (UCameraComponent* Camera = Actor->GetComponentByClass<UCameraComponent>())
        {
            int32 Id = sceneData.NextUUID++;
            sceneData.Cameras[Id] = Camera;
        }
    }

    return sceneData;
}

