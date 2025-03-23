#pragma once
#include <sstream>

#include "Define.h"
#include "Container/Map.h"
#include "UObject/ObjectMacros.h"
#include "ViewportClient.h"
#include "EngineLoop.h"


#define MIN_ORTHOZOOM				1.0							/* 2D ortho viewport zoom >= MIN_ORTHOZOOM */
#define MAX_ORTHOZOOM				1e25	

extern FEngineLoop GEngineLoop;



struct FViewportCameraTransform
{
private:

public:

    FVector GetForwardVector();
    FVector GetRightVector();
public:
    FViewportCameraTransform();

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

public:
    /** Current viewport Position. */
    FVector	ViewLocation;
    /** Current Viewport orientation; valid only for perspective projections. */
    FVector ViewRotation;
    FVector	DesiredLocation;
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
    ~FEditorViewportClient();

    virtual void        Draw(FViewport* Viewport) override;
    virtual UWorld*     GetWorld() const { return NULL; };
    void Initialize(int32 viewportIndex);
    void Tick(float DeltaTime);
    void Release();
    void LoadConfig();
    void SaveConfig();
    void Input();
    void ResizeViewport(const DXGI_SWAP_CHAIN_DESC& swapchaindesc);
    void ResizeViewport(FRect Top, FRect Bottom, FRect Left, FRect Right);

    bool IsSelected(POINT point);
protected:
    /** Camera speed setting */
    int32 CameraSpeedSetting = 1;
    /** Camera speed scalar */
    float CameraSpeedScalar = 1.0f;
    float GridSize;

public: 
    FViewport* Viewport;
    int32 ViewportIndex;
    FViewport* GetViewport() { return Viewport; }
    D3D11_VIEWPORT& GetD3DViewport();


public:
    //카메라
    /** Viewport camera transform data for perspective viewports */
    FViewportCameraTransform		ViewTransformPerspective;

    // 카메라 정보 
    float ViewFOV = 60.0f;
    /** Viewport's stored horizontal field of view (saved in ini files). */
    float FOVAngle = 60.0f;
    float AspectRatio;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    FMatrix View;
    FMatrix Projection;
public: //Camera Movement
    void CameraMoveForward(float _Value);
    void CameraMoveRigth(float _Value);
    void CameraMoveUp(float _Value);
    void CameraRotateYaw(float _Value);
    void CameraRotatePitch(float _Value);

    FMatrix& GetViewMatrix() { return  View; }
    FMatrix& GetProjectionMatrix() { return Projection; }
    void UpdateViewMatrix();
    void UpdateProjectionMatrix();
private: // Input
    POINT lastMousePos;
    bool bRightMouseDown = false;


private :
    const FString IniFilePath = "editor.ini";
private:
    TMap<FString, FString> ReadIniFile(const FString& filePath);
    void WriteIniFile(const FString& filePath, const TMap<FString, FString>& config);
	
public:
    PROPERTY(int32, CameraSpeedSetting)
    PROPERTY(float, GridSize)
    float GetCameraSpeedScalar() const { return CameraSpeedScalar; };
    void SetCameraSpeedScalar(float value);

private:
    template <typename T>
    T GetValueFromConfig(const TMap<FString, FString>& config, const FString& key, T defaultValue) {
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

