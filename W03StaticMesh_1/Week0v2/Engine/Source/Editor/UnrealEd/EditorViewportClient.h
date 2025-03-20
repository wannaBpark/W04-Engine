#pragma once
#include "Define.h"

class FEditorViewportClient
{
public:
    FEditorViewportClient();
    ~FEditorViewportClient();

    void Initialize();
    void Release();
    void LoadConfig();
    void SaveConfig();
protected:
    /** Camera speed setting */
    int32 CameraSpeedSetting = 1;
    /** Camera speed scalar */
    float CameraSpeedScalar = 1.0f;
    float GridSize;

private :
    const FString IniFilePath = "editor.ini";
private:
    TMap<FString, FString> ReadIniFile(const FString& filePath);
    void WriteIniFile(const FString& filePath, const std::unordered_map<FString, FString>& config);
	
public:
    PROPERTY(int32, CameraSpeedSetting)
    PROPERTY(float, GridSize)
    float GetCameraSpeedScalar(){ return CameraSpeedScalar; };
    void SetCameraSpeedScalar(float value);
private:
    template <typename T>
    T GetValueFromConfig(const std::unordered_map<std::string, std::string>& config, const std::string& key, T defaultValue) {
        auto it = config.find(key);
        if (it != config.end()) {
            std::istringstream ss(it->second);
            T value;
            if (ss >> value) {
                return value;
            }
        }
        return defaultValue;
    }
};

