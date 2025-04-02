#pragma once
#include <sstream>

#include "Define.h"
#include "Container/Map.h"
#include "UObject/ObjectMacros.h"
#include "ViewportClient.h"
#include "EngineLoop.h"
#include "EngineBaseTypes.h"

#define MIN_ORTHOZOOM				1.0							/* 2D ortho viewport zoom >= MIN_ORTHOZOOM */
#define MAX_ORTHOZOOM				1e25

extern FEngineLoop GEngineLoop;


struct FViewportCameraTransform
{
public:
    FVector GetForwardVector() const;
    FVector GetRightVector() const;
    FVector GetUpVector() const;

    FViewportCameraTransform() = default;

    /** Sets the transform's location */
    void SetLocation(const FVector& Position)
    {
        ViewLocation = Position;
    }

    /** Sets the transform's rotation */
    void SetRotation(const FVector& Rotation)
    {
        ViewRotation = Rotation;
    }

    /** Sets the location to look at during orbit */
    void SetLookAt(const FVector& InLookAt)
    {
        LookAt = InLookAt;
    }

    /** Set the ortho zoom amount */
    void SetOrthoZoom(float InOrthoZoom)
    {
        assert(InOrthoZoom >= MIN_ORTHOZOOM && InOrthoZoom <= MAX_ORTHOZOOM);
        OrthoZoom = InOrthoZoom;
    }

    /** Check if transition curve is playing. */
    /*    bool IsPlaying();*/

    /** @return The transform's location */
    FORCEINLINE const FVector& GetLocation() const { return ViewLocation; }

    /** @return The transform's rotation */
    FORCEINLINE const FVector& GetRotation() const { return ViewRotation; }

    /** @return The look at point for orbiting */
    FORCEINLINE const FVector& GetLookAt() const { return LookAt; }

    /** @return The ortho zoom amount */
    FORCEINLINE float GetOrthoZoom() const { return OrthoZoom; }

    /** Current viewport Position. */
    FVector ViewLocation;
    /** Current Viewport orientation; valid only for perspective projections. */
    FVector ViewRotation;
    FVector DesiredLocation;
    /** When orbiting, the point we are looking at */
    FVector LookAt;
    /** Viewport start location when animating to another location */
    FVector StartLocation;
    /** Ortho zoom amount */
    float OrthoZoom;
};

class FEditorViewportClient : public FViewportClient
{
public:
    FEditorViewportClient();
    virtual ~FEditorViewportClient() override;

    virtual void Draw(FViewport* Viewport) override;
    void Initialize(int32 viewportIndex);
    void Tick(float DeltaTime);
    void Release();

    void Input();
    void ResizeViewport(const DXGI_SWAP_CHAIN_DESC& swapchaindesc);
    void ResizeViewport(FRect Top, FRect Bottom, FRect Left, FRect Right);

    bool IsSelected(POINT point) const;

protected:
    /** Camera speed setting */
    int32 CameraSpeedSetting = 1;
    /** Camera speed scalar */
    float CameraSpeedScalar = 1.0f;
    float GridSize;

public:
    FViewport* Viewport = nullptr;
    int32 ViewportIndex = 0;
    FViewport* GetViewport() const { return Viewport; }
    D3D11_VIEWPORT& GetD3DViewport() const;


    //카메라
    /** Viewport camera transform data for perspective viewports */
    FViewportCameraTransform ViewTransformPerspective{};
    FViewportCameraTransform ViewTransformOrthographic{};
    // 카메라 정보 
    float ViewFOV = 60.0f;
    /** Viewport's stored horizontal field of view (saved in ini files). */
    float FOVAngle = 60.0f;
    float AspectRatio;
    float nearPlane = 0.1f;
    //float farPlane = 1000000.0f;
    float farPlane = D3D11_FLOAT32_MAX;
    static FVector Pivot;
    static float orthoSize;
    ELevelViewportType ViewportType;
    uint64 ShowFlag;
    EViewModeIndex ViewMode;

    FMatrix View;
    FMatrix Projection;

    FFrustum Frustum;
    //Camera Movement
    void CameraMoveForward(float _Value);
    void CameraMoveRight(float _Value);
    void CameraMoveUp(float _Value);
    void CameraRotateYaw(float _Value);
    void CameraRotatePitch(float _Value);
    void PivotMoveRight(float _Value) const;
    void PivotMoveUp(float _Value) const;

    FMatrix& GetViewMatrix() { return View; }
    FMatrix& GetProjectionMatrix() { return Projection; }
    void UpdateViewMatrix();
    void UpdateProjectionMatrix();

    bool IsOrtho() const;
    bool IsPerspective() const;
    ELevelViewportType GetViewportType() const;
    void SetViewportType(ELevelViewportType InViewportType);
    void UpdateOrthoCameraLoc();
    EViewModeIndex GetViewMode() const { return ViewMode; }
    void SetViewMode(EViewModeIndex newMode) { ViewMode = newMode; }
    uint64 GetShowFlag() const { return ShowFlag; }
    void SetShowFlag(uint64 newMode) { ShowFlag = newMode; }
    bool GetIsOnRBMouseClick() const { return bRightMouseDown; }

    //Flag Test Code
    static void SetOthoSize(float _Value);

private: // Input
    POINT lastMousePos;
    bool bRightMouseDown = false;

public:
    void LoadConfig(const TMap<FString, FString>& config);
    void SaveConfig(TMap<FString, FString>& config) const;

private:
    TMap<FString, FString> ReadIniFile(const FString& filePath) const;
    void WriteIniFile(const FString& filePath, const TMap<FString, FString>& config) const;

public:
    PROPERTY(int32, CameraSpeedSetting)
    PROPERTY(float, GridSize)
    float GetCameraSpeedScalar() const { return CameraSpeedScalar; };
    void SetCameraSpeedScalar(float value);

    FFrustum CreateFrustumFromCamera() const;

private:
    template <typename T>
    T GetValueFromConfig(const TMap<FString, FString>& config, const FString& key, T defaultValue)
    {
        if (const FString* Value = config.Find(key))
        {
            std::istringstream iss(**Value);
            T value;
            if (iss >> value)
            {
                return value;
            }
        }
        return defaultValue;
    }
};
