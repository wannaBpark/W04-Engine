#include "UnrealEd/SceneMgr.h"
#include "JSON/json.hpp"
#include "UObject/Object.h"
#include "Components/SphereComp.h"
#include "Components/CubeComp.h"
#include "BaseGizmos/GizmoArrowComponent.h"
#include "UObject/ObjectFactory.h"
#include <fstream>
#include "Components/UBillboardComponent.h"
#include "Components/LightComponent.h"
#include "Components/SkySphereComponent.h"
#include "Camera/CameraComponent.h"
#include "UObject/Casts.h"
#include <tinyfiledialogs/tinyfiledialogs.h>
#include "Editor/LevelEditor/SLevelEditor.h"
#include "Editor/UnrealEd/EditorViewportClient.h"
#include <Engine/FLoaderOBJ.h>
#include <Engine/StaticMeshActor.h>
#include "Runtime/Engine/World.h"
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
        if (jsonData.contains("PerspectiveCamera"))
        {
            auto camData = jsonData["PerspectiveCamera"];
            FVector Location(camData["Location"][0], camData["Location"][1], camData["Location"][2]);
            FVector Rotation(camData["Rotation"][0], camData["Rotation"][1], camData["Rotation"][2]);
            float FOV = camData["FOV"][0];

            auto Viewport = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();
            Viewport->ViewTransformPerspective.SetLocation(Location);
            Viewport->ViewTransformPerspective.SetRotation(Rotation);
            Viewport->ViewFOV = FOV;
        }
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
                }

            }
        }
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
    j["Version"] = sceneData.Version;
    j["NextUUID"] = sceneData.NextUUID;

    // Primitives 처리 (C++17 스타일)
    for (const auto& [Id, Obj] : sceneData.Primitives)
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
        j["Primitives"][std::to_string(Id)] = {
            {"Location", Location},
            {"Rotation", Rotation},
            {"Scale", Scale},
            {"Type",primitiveName}
        };
    }

    for (const auto& [id, camera] : sceneData.Cameras)
    {
        UCameraComponent* cameraComponent = static_cast<UCameraComponent*>(camera);
        TArray<float> Location = { cameraComponent->GetWorldLocation().x, cameraComponent->GetWorldLocation().y, cameraComponent->GetWorldLocation().z };
        TArray<float> Rotation = { 0.0f, cameraComponent->GetWorldRotation().y, cameraComponent->GetWorldRotation().z };
        float FOV = cameraComponent->GetFOV();
        float nearClip = cameraComponent->GetNearClip();
        float farClip = cameraComponent->GetFarClip();
    
        //
        j["PerspectiveCamera"][std::to_string(id)] = {
            {"Location", Location},
            {"Rotation", Rotation},
            {"FOV", FOV},
            {"NearClip", nearClip},
            {"FarClip", farClip}
        };
    }


    return j.dump(4); // 4는 들여쓰기 수준
}

// SceneData를 json으로 직렬화하여 파일에 저장.
bool FSceneMgr::SaveSceneToFile(const FString& filename, const SceneData& sceneData)
{
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

