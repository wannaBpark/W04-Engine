#include "EditorViewportClient.h"
#include "fstream"
#include "sstream"
#include "ostream"
#include "Math/JungleMath.h"
#include "EngineLoop.h"
#include "UnrealClient.h"

FEditorViewportClient::FEditorViewportClient()
    : Viewport(nullptr)
{

}

FEditorViewportClient::~FEditorViewportClient()
{
    Release();
}

void FEditorViewportClient::Draw(FViewport* Viewport)
{
}

void FEditorViewportClient::Initialize(int32 viewportIndex)
{
    LoadConfig();
    ViewTransformPerspective.SetLocation(FVector(8.0f, 8.0f, 8.f));
    ViewTransformPerspective.SetRotation(FVector(0.0f, 45.0f, -135.0f));
    Viewport = new FViewport(static_cast<EViewScreenLocation>(viewportIndex));
    ResizeViewport(GEngineLoop.graphicDevice.SwapchainDesc);
    ViewportIndex = viewportIndex;
}

void FEditorViewportClient::Tick(float DeltaTime)
{
    Input();
    UpdateViewMatrix();
    UpdateProjectionMatrix();

}

void FEditorViewportClient::Release()
{
    if (Viewport)
        delete Viewport;
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

void FEditorViewportClient::Input()
{
    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) // VK_RBUTTON은 마우스 오른쪽 버튼을 나타냄
    {
        if (!bRightMouseDown)
        {
            // 마우스 오른쪽 버튼을 처음 눌렀을 때, 마우스 위치 초기화
            GetCursorPos(&lastMousePos);
            bRightMouseDown = true;
        }
        else
        {
            // 마우스 이동량 계산
            POINT currentMousePos;
            GetCursorPos(&currentMousePos);

            // 마우스 이동 차이 계산
            int32 deltaX = currentMousePos.x - lastMousePos.x;
            int32 deltaY = currentMousePos.y - lastMousePos.y;

            // Yaw(좌우 회전) 및 Pitch(상하 회전) 값 변경
            CameraRotateYaw(deltaX * 0.1f);  // X 이동에 따라 좌우 회전
            CameraRotatePitch(deltaY * 0.1f);  // Y 이동에 따라 상하 회전

            // 새로운 마우스 위치 저장
            lastMousePos = currentMousePos;
        }
        if (GetAsyncKeyState('A') & 0x8000)
        {
            CameraMoveRigth(-1.f);
        }
        if (GetAsyncKeyState('D') & 0x8000)
        {
            CameraMoveRigth(1.f);
        }
        if (GetAsyncKeyState('W') & 0x8000)
        {
            CameraMoveForward(1.f);
        }
        if (GetAsyncKeyState('S') & 0x8000)
        {
            CameraMoveForward(-1.f);
        }
        if (GetAsyncKeyState('E') & 0x8000)
        {
            CameraMoveUp(1.f);
        }
        if (GetAsyncKeyState('Q') & 0x8000)
        {
            CameraMoveUp(-1.f);
        }
    }
    else
    {
        bRightMouseDown = false; // 마우스 오른쪽 버튼을 떼면 상태 초기화
    }
}
void FEditorViewportClient::ResizeViewport(const DXGI_SWAP_CHAIN_DESC& swapchaindesc)
{
    if (Viewport) { 
        Viewport->ResizeViewport(swapchaindesc);    
    }
    else {
        UE_LOG(LogLevel::Error, "Viewport is nullptr");
    }
    AspectRatio = GEngineLoop.GetAspectRatio(GEngineLoop.graphicDevice.SwapChain);
    UpdateProjectionMatrix();
    UpdateViewMatrix();
}
void FEditorViewportClient::ResizeViewport(FRect Top, FRect Bottom, FRect Left, FRect Right)
{
    if (Viewport) {
        Viewport->ResizeViewport(Top, Bottom, Left, Right);
    }
    else {
        UE_LOG(LogLevel::Error, "Viewport is nullptr");
    }
    AspectRatio = GEngineLoop.GetAspectRatio(GEngineLoop.graphicDevice.SwapChain);
    UpdateProjectionMatrix();
    UpdateViewMatrix();
}
bool FEditorViewportClient::IsSelected(POINT point)
{
    float TopLeftX = Viewport->GetViewport().TopLeftX;
    float TopLeftY = Viewport->GetViewport().TopLeftY;
    float Width = Viewport->GetViewport().Width;
    float Height = Viewport->GetViewport().Height;

    if (point.x >= TopLeftX && point.x <= TopLeftX + Width &&
        point.y >= TopLeftY && point.y <= TopLeftY + Height)
    {
        return true;
    }
    return false;
}
D3D11_VIEWPORT& FEditorViewportClient::GetD3DViewport()
{
    return Viewport->GetViewport();
}
void FEditorViewportClient::CameraMoveForward(float _Value)
{
    FVector curCameraLoc = ViewTransformPerspective.GetLocation();
    curCameraLoc = curCameraLoc +  ViewTransformPerspective.GetForwardVector() * GetCameraSpeedScalar() * _Value;
    ViewTransformPerspective.SetLocation(curCameraLoc);
}

void FEditorViewportClient::CameraMoveRigth(float _Value)
{
    FVector curCameraLoc = ViewTransformPerspective.GetLocation();
    curCameraLoc = curCameraLoc + ViewTransformPerspective.GetRightVector() * GetCameraSpeedScalar() * _Value;
    ViewTransformPerspective.SetLocation(curCameraLoc);
}

void FEditorViewportClient::CameraMoveUp(float _Value)
{
    FVector curCameraLoc = ViewTransformPerspective.GetLocation();
    curCameraLoc.z = curCameraLoc.z +  GetCameraSpeedScalar() * _Value;
    ViewTransformPerspective.SetLocation(curCameraLoc);
}

void FEditorViewportClient::CameraRotateYaw(float _Value)
{
    FVector curCameraRot = ViewTransformPerspective.GetRotation();
    curCameraRot.z += _Value ;
    ViewTransformPerspective.SetRotation(curCameraRot);
}

void FEditorViewportClient::CameraRotatePitch(float _Value)
{
    FVector curCameraRot = ViewTransformPerspective.GetRotation();
    curCameraRot.y += _Value;
    if (curCameraRot.y < -90.0f)
        curCameraRot.y = -90.0f;
    if (curCameraRot.y > 90.0f)
        curCameraRot.y = 90.0f;
    ViewTransformPerspective.SetRotation(curCameraRot);
}

void FEditorViewportClient::UpdateViewMatrix()
{
    View = JungleMath::CreateViewMatrix(ViewTransformPerspective.GetLocation(),
        ViewTransformPerspective.GetLocation() + ViewTransformPerspective.GetForwardVector(),
        FVector{ 0.0f,0.0f, 1.0f });
}

void FEditorViewportClient::UpdateProjectionMatrix()
{
    Projection = JungleMath::CreateProjectionMatrix(
        ViewFOV * (3.141592f / 180.0f),
        GEngineLoop.GetAspectRatio(GEngineLoop.graphicDevice.SwapChain),
        nearPlane,
        farPlane
    );
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


FVector FViewportCameraTransform::GetForwardVector()
{
    FVector Forward = FVector(1.f, 0.f, 0.0f);
    Forward = JungleMath::FVectorRotate(Forward, ViewRotation);
    return Forward;
}
FVector FViewportCameraTransform::GetRightVector()
{
    FVector Right = FVector(0.f, 1.f, 0.0f);
	Right = JungleMath::FVectorRotate(Right, ViewRotation);
	return Right;
}

FViewportCameraTransform::FViewportCameraTransform()
{
}
