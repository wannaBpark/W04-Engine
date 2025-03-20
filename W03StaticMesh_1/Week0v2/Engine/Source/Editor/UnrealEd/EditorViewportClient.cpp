#include "Engine/Source/Editor/UnrealEd/EditorViewportClient.h"
#include "fstream"
#include "sstream"
#include "ostream"
FEditorViewportClient::FEditorViewportClient()
{
    Initialize();
}

FEditorViewportClient::~FEditorViewportClient()
{
    Release();
}

void FEditorViewportClient::Initialize()
{
    LoadConfig();
}

void FEditorViewportClient::Release()
{
    SaveConfig();
}

void FEditorViewportClient::LoadConfig()
{
    auto config = ReadIniFile(IniFilePath);
    CameraSpeedSetting = GetValueFromConfig(config, "CameraSpeedSetting", 1);
    CameraSpeedScalar = GetValueFromConfig(config, "CameraSpeedScalar", 1.0f);
    GridSize = GetValueFromConfig(config, "GridSize", 10.0f);
}

void FEditorViewportClient::SaveConfig()
{
    TMap<FString, FString> config;
    config["CameraSpeedSetting"] = std::to_string(CameraSpeedSetting);
    config["CameraSpeedScalar"] = std::to_string(CameraSpeedScalar);
    config["GridSize"] = std::to_string(GridSize);
    WriteIniFile(IniFilePath, config);
}

TMap<FString, FString> FEditorViewportClient::ReadIniFile(const FString& filePath)
{
    TMap<FString, FString> config;
    std::ifstream file(*filePath);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '[' || line[0] == ';') continue;
        std::istringstream ss(line);
        std::string key, value;
        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
            config[key] = value;
        }
    }
    return config;
}

void FEditorViewportClient::WriteIniFile(const FString& filePath, const TMap<FString, FString>& config)
{
    std::ofstream file(*filePath);
    for (const auto& pair : config) {
        file << *pair.Key << "=" << *pair.Value << "\n";
    }
}

void FEditorViewportClient::SetCameraSpeedScalar(float value)
{
    if (value < 0.198f)
        value = 0.198f;
    else if (value > 176.0f)
        value = 176.0f;
    CameraSpeedScalar = value;
}
