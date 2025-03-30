#include "EditorViewportClient.h"
#include "fstream"
#include "sstream"
#include "ostream"
#include "Math/JungleMath.h"
#include "EngineLoop.h"
#include "UnrealClient.h"
#include "World.h"
#include "GameFramework/Actor.h"

FVector FEditorViewportClient::Pivot = FVector(0.0f, 0.0f, 0.0f);
float FEditorViewportClient::orthoSize = 10.0f;
FEditorViewportClient::FEditorViewportClient()
    : Viewport(nullptr), ViewMode(VMI_Lit), ViewportType(LVT_Perspective), ShowFlag(31)
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
 
}



void FEditorViewportClient::Input()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) // VK_RBUTTON은 마우스 오른쪽 버튼을 나타냄
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_None);
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
            if (IsPerspective()) {
                CameraRotateYaw(deltaX * 0.1f);  // X 이동에 따라 좌우 회전
                CameraRotatePitch(deltaY * 0.1f);  // Y 이동에 따라 상하 회전
            }
            else
            {
                PivotMoveRight(deltaX);
                PivotMoveUp(deltaY);
            }

            SetCursorPos(lastMousePos.x, lastMousePos.y);
        }
        if (GetAsyncKeyState('A') & 0x8000)
        {
            CameraMoveRight(-1.f);
        }
        if (GetAsyncKeyState('D') & 0x8000)
        {
            CameraMoveRight(1.f);
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
        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
        bRightMouseDown = false; // 마우스 오른쪽 버튼을 떼면 상태 초기화
    }

    // Focus Selected Actor
    if (GetAsyncKeyState('F') & 0x8000)
    {
        if (AActor* PickedActor = GEngineLoop.GetWorld()->GetSelectedActor())
        {
            FViewportCameraTransform& ViewTransform = ViewTransformPerspective;
            ViewTransform.SetLocation(
                // TODO: 10.0f 대신, 정점의 min, max의 거리를 구해서 하면 좋을 듯
                PickedActor->GetActorLocation() - (ViewTransform.GetForwardVector() * 10.0f)
            );
        }
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
    if (IsPerspective()) {
        FVector curCameraLoc = ViewTransformPerspective.GetLocation();
        curCameraLoc = curCameraLoc + ViewTransformPerspective.GetForwardVector() * GetCameraSpeedScalar() * _Value;
        ViewTransformPerspective.SetLocation(curCameraLoc);
    }
    else
    {
        Pivot.X += _Value * 0.1f;
    }
}

void FEditorViewportClient::CameraMoveRight(float _Value)
{
    if (IsPerspective()) {
        FVector curCameraLoc = ViewTransformPerspective.GetLocation();
        curCameraLoc = curCameraLoc + ViewTransformPerspective.GetRightVector() * GetCameraSpeedScalar() * _Value;
        ViewTransformPerspective.SetLocation(curCameraLoc);
    }
    else
    {
        Pivot.Y += _Value * 0.1f;
    }
}

void FEditorViewportClient::CameraMoveUp(float _Value)
{
    if (IsPerspective()) {
        FVector curCameraLoc = ViewTransformPerspective.GetLocation();
        curCameraLoc.Z = curCameraLoc.Z + GetCameraSpeedScalar() * _Value;
        ViewTransformPerspective.SetLocation(curCameraLoc);
    }
    else {
        Pivot.Z += _Value * 0.1f;
    }
}

void FEditorViewportClient::CameraRotateYaw(float _Value)
{
    FVector curCameraRot = ViewTransformPerspective.GetRotation();
    curCameraRot.Z += _Value ;
    ViewTransformPerspective.SetRotation(curCameraRot);
}

void FEditorViewportClient::CameraRotatePitch(float _Value)
{
    FVector curCameraRot = ViewTransformPerspective.GetRotation();
    curCameraRot.Y += _Value;
    if (curCameraRot.Y <= -89.0f)
        curCameraRot.Y = -89.0f;
    if (curCameraRot.Y >= 89.0f)
        curCameraRot.Y = 89.0f;
    ViewTransformPerspective.SetRotation(curCameraRot);
}

void FEditorViewportClient::PivotMoveRight(float _Value)
{
    Pivot = Pivot + ViewTransformOrthographic.GetRightVector() * _Value * -0.05f;
}

void FEditorViewportClient::PivotMoveUp(float _Value)
{
    Pivot = Pivot + ViewTransformOrthographic.GetUpVector() * _Value * 0.05f;
}

void FEditorViewportClient::UpdateViewMatrix()
{
    if (IsPerspective()) {
        View = JungleMath::CreateViewMatrix(ViewTransformPerspective.GetLocation(),
            ViewTransformPerspective.GetLocation() + ViewTransformPerspective.GetForwardVector(),
            FVector{ 0.0f,0.0f, 1.0f });
    }
    else 
    {
        UpdateOrthoCameraLoc();
        if (ViewportType == LVT_OrthoXY || ViewportType == LVT_OrthoNegativeXY) {
            View = JungleMath::CreateViewMatrix(ViewTransformOrthographic.GetLocation(),
                Pivot, FVector(0.0f, -1.0f, 0.0f));
        }
        else
        {
            View = JungleMath::CreateViewMatrix(ViewTransformOrthographic.GetLocation(),
                Pivot, FVector(0.0f, 0.0f, 1.0f));
        }
    }
}

void FEditorViewportClient::UpdateProjectionMatrix()
{
    if (IsPerspective()) {
        Projection = JungleMath::CreateProjectionMatrix(
            ViewFOV * (3.141592f / 180.0f),
            GetViewport()->GetViewport().Width/ GetViewport()->GetViewport().Height,
            nearPlane,
            farPlane
        );
    }
    else
    {
        // 스왑체인의 가로세로 비율을 구합니다.
        float aspectRatio = GetViewport()->GetViewport().Width / GetViewport()->GetViewport().Height;

        // 오쏘그래픽 너비는 줌 값과 가로세로 비율에 따라 결정됩니다.
        float orthoWidth = orthoSize * aspectRatio;
        float orthoHeight = orthoSize;

        // 오쏘그래픽 투영 행렬 생성 (nearPlane, farPlane 은 기존 값 사용)
        Projection = JungleMath::CreateOrthoProjectionMatrix(
            orthoWidth,
            orthoHeight,
            nearPlane,
            farPlane
        );
    }
}

bool FEditorViewportClient::IsOrtho() const
{
    return !IsPerspective();
}

bool FEditorViewportClient::IsPerspective() const
{
    return (GetViewportType() == LVT_Perspective);
}

ELevelViewportType FEditorViewportClient::GetViewportType() const
{
    ELevelViewportType EffectiveViewportType = ViewportType;
    if (EffectiveViewportType == LVT_None)
    {
        EffectiveViewportType = LVT_Perspective;
    }
    //if (bUseControllingActorViewInfo)
    //{
    //    EffectiveViewportType = (ControllingActorViewInfo.ProjectionMode == ECameraProjectionMode::Perspective) ? LVT_Perspective : LVT_OrthoFreelook;
    //}
    return EffectiveViewportType;
}

void FEditorViewportClient::SetViewportType(ELevelViewportType InViewportType)
{
    ViewportType = InViewportType;
    //ApplyViewMode(GetViewMode(), IsPerspective(), EngineShowFlags);

    //// We might have changed to an orthographic viewport; if so, update any viewport links
    //UpdateLinkedOrthoViewports(true);

    //Invalidate();
}

void FEditorViewportClient::UpdateOrthoCameraLoc()
{
    switch (ViewportType)
    {
    case LVT_OrthoXY: // Top
        ViewTransformOrthographic.SetLocation(Pivot + FVector::UpVector*100000.0f);
        ViewTransformOrthographic.SetRotation(FVector(0.0f, 90.0f, -90.0f));
        break;
    case LVT_OrthoXZ: // Front
        ViewTransformOrthographic.SetLocation(Pivot + FVector::ForwardVector*100000.0f);
        ViewTransformOrthographic.SetRotation(FVector(0.0f, 0.0f, 180.0f));
        break;
    case LVT_OrthoYZ: // Left
        ViewTransformOrthographic.SetLocation(Pivot + FVector::RightVector*100000.0f);
        ViewTransformOrthographic.SetRotation(FVector(0.0f, 0.0f, 270.0f));
        break;
    case LVT_Perspective:
        break;
    case LVT_OrthoNegativeXY: // Bottom
        ViewTransformOrthographic.SetLocation(Pivot + FVector::UpVector * -1.0f*100000.0f);
        ViewTransformOrthographic.SetRotation(FVector(0.0f, -90.0f, 90.0f));
        break;
    case LVT_OrthoNegativeXZ: // Back
        ViewTransformOrthographic.SetLocation(Pivot + FVector::ForwardVector * -1.0f*100000.0f);
        ViewTransformOrthographic.SetRotation(FVector(0.0f, 0.0f, 0.0f));
        break;
    case LVT_OrthoNegativeYZ: // Right
        ViewTransformOrthographic.SetLocation(Pivot + FVector::RightVector * -1.0f*100000.0f);
        ViewTransformOrthographic.SetRotation(FVector(0.0f, 0.0f, 90.0f));
        break;
    case LVT_MAX:
        break;
    case LVT_None:
        break;
    default:
        break;
    }
}

void FEditorViewportClient::SetOthoSize(float _Value)
{
    orthoSize += _Value;
    if (orthoSize <= 0.1f)
        orthoSize = 0.1f;
    
}

void FEditorViewportClient::LoadConfig(const TMap<FString, FString>& config)
{
    FString ViewportNum = std::to_string(ViewportIndex);
    CameraSpeedSetting = GetValueFromConfig(config, "CameraSpeedSetting" + ViewportNum, 1);
    CameraSpeedScalar = GetValueFromConfig(config, "CameraSpeedScalar" + ViewportNum, 1.0f);
    GridSize = GetValueFromConfig(config, "GridSize"+ ViewportNum, 10.0f);
    ViewTransformPerspective.ViewLocation.X = GetValueFromConfig(config, "PerspectiveCameraLocX" + ViewportNum, 0.0f);
    ViewTransformPerspective.ViewLocation.Y = GetValueFromConfig(config, "PerspectiveCameraLocY" + ViewportNum, 0.0f);
    ViewTransformPerspective.ViewLocation.Z = GetValueFromConfig(config, "PerspectiveCameraLocZ" + ViewportNum, 0.0f);
    ViewTransformPerspective.ViewRotation.X = GetValueFromConfig(config, "PerspectiveCameraRotX" + ViewportNum, 0.0f);
    ViewTransformPerspective.ViewRotation.Y = GetValueFromConfig(config, "PerspectiveCameraRotY" + ViewportNum, 0.0f);
    ViewTransformPerspective.ViewRotation.Z = GetValueFromConfig(config, "PerspectiveCameraRotZ" + ViewportNum, 0.0f);
    ShowFlag = GetValueFromConfig(config, "ShowFlag" + ViewportNum, 31.0f);
    ViewMode = static_cast<EViewModeIndex>(GetValueFromConfig(config, "ViewMode" + ViewportNum, 0));
    ViewportType = static_cast<ELevelViewportType>(GetValueFromConfig(config, "ViewportType" + ViewportNum, 3));
}
void FEditorViewportClient::SaveConfig(TMap<FString, FString>& config)
{
    FString ViewportNum = std::to_string(ViewportIndex);
    config["CameraSpeedSetting"+ ViewportNum] = std::to_string(CameraSpeedSetting);
    config["CameraSpeedScalar"+ ViewportNum] = std::to_string(CameraSpeedScalar);
    config["GridSize"+ ViewportNum] = std::to_string(GridSize);
    config["PerspectiveCameraLocX" + ViewportNum] = std::to_string(ViewTransformPerspective.GetLocation().X);
    config["PerspectiveCameraLocY" + ViewportNum] = std::to_string(ViewTransformPerspective.GetLocation().Y);
    config["PerspectiveCameraLocZ" + ViewportNum] = std::to_string(ViewTransformPerspective.GetLocation().Z);
    config["PerspectiveCameraRotX" + ViewportNum] = std::to_string(ViewTransformPerspective.GetRotation().X);
    config["PerspectiveCameraRotY" + ViewportNum] = std::to_string(ViewTransformPerspective.GetRotation().Y);
    config["PerspectiveCameraRotZ" + ViewportNum] = std::to_string(ViewTransformPerspective.GetRotation().Z);
    config["ShowFlag"+ ViewportNum] = std::to_string(ShowFlag);
    config["ViewMode" + ViewportNum] = std::to_string(int32(ViewMode));
    config["ViewportType" + ViewportNum] = std::to_string(int32(ViewportType));
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

FVector FViewportCameraTransform::GetUpVector()
{
    FVector Up = FVector(0.f, 0.f, 1.0f);
    Up = JungleMath::FVectorRotate(Up, ViewRotation);
    return Up;
}

FViewportCameraTransform::FViewportCameraTransform()
{
}

FFrustum FEditorViewportClient::CreateFrustumFromCamera()
{
    FFrustum Frustum;
    auto& Camera = ViewTransformPerspective;
    FVector CamPos = Camera.GetLocation();
    FVector Forward = Camera.GetForwardVector().GetSafeNormal(); // 카메라 정면 방향 (+Z)
    FVector Right = Camera.GetRightVector().GetSafeNormal();   // 카메라 우측 방향 (+X)
    FVector Up = Camera.GetUpVector().GetSafeNormal();      // 카메라 상측 방향 (+Y)

    constexpr float FarDist = 100.0f;             // TODO : 변경 필요!!!!!!
    const float& FOV = FOVAngle;
    const float& NearDist = nearPlane;

    const float HalfV = tanf(FMath::DegreesToRadians(FOV) * 0.5f) * NearDist;
    const float HalfH = HalfV * AspectRatio;

    const FVector NearCenter = CamPos + Forward * NearDist;
    const FVector FarCenter = CamPos + Forward * FarDist;

    const FVector Back = Forward * -1.0f;
    // Near / Far 평면
    Frustum.Planes[4] = FPlane(Forward, NearCenter);  // Near
    Frustum.Planes[5] = FPlane(Back, FarCenter);  // Far

    // Left 평면
    FVector LeftNormal = (NearCenter - Right * HalfH - CamPos).GetSafeNormal();
    Frustum.Planes[0] = FPlane(Up.Cross(LeftNormal), CamPos);

    // Right 평면
    FVector RightNormal = (NearCenter + Right * HalfH - CamPos).GetSafeNormal();
    Frustum.Planes[1] = FPlane(RightNormal.Cross(Up), CamPos);

    // Top 평면
    FVector TopNormal = (NearCenter + Up * HalfV - CamPos).GetSafeNormal();
    Frustum.Planes[2] = FPlane(Right.Cross(TopNormal), CamPos);
    /*FVector TopNormal = (NearCenter + Up * HalfV - CamPos).GetSafeNormal();
    Frustum.Planes[2] = FPlane(TopNormal.Cross(Right), CamPos);*/

    // Bottom 평면
    FVector BottomNormal = (NearCenter - Up * HalfV - CamPos).GetSafeNormal();
    Frustum.Planes[3] = FPlane(BottomNormal.Cross(Right), CamPos);
    /*FVector BottomNormal = (NearCenter - Up * HalfV - CamPos).GetSafeNormal();
    Frustum.Planes[3] = FPlane(Right.Cross(BottomNormal), CamPos);*/

    // 출력용 로그 (평면의 normal 방향 체크용)
    /*const char* name[6] = {
        "Left", "Right", "Top", "Bottom", "Near", "Far"
    };
    for (int i = 0; i < 6; ++i) {
        UE_LOG(LogLevel::Display, "Plane No %s : %.2f %.2f %.2f", name[i],
            Frustum.Planes[i].Normal.X, Frustum.Planes[i].Normal.Y, Frustum.Planes[i].Normal.Z);
    }*/
    return Frustum;
}
