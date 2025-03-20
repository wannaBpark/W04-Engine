#pragma once
#include "Define.h"

class UObject;
struct SceneData {
    int32 Version;
    int32 NextUUID;
    TMap<int32, UObject*> Primitives;
    TMap<int32, UObject*> Cameras;
};
class FSceneMgr
{
public:
    static SceneData ParseSceneData(const std::string& jsonStr);
    static FString LoadSceneFromFile(const std::string& filename);
    static std::string SerializeSceneData(const SceneData& sceneData);
    static bool SaveSceneToFile(const std::string& filename, const SceneData& sceneData);
};

