#include "CameraComponent.h"
#include "Math/JungleMath.h"
#include "Engine/World.h"
#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"

UCameraComponent::UCameraComponent()
{
}

UCameraComponent::~UCameraComponent()
{
}

void UCameraComponent::InitializeComponent()
{
	Super::InitializeComponent();
	RelativeLocation = FVector(0.0f, 0.0f, 0.5f);
	FOV = 60.f;
}

void UCameraComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

	Input();
	QuatRotation = JungleMath::EulerToQuaternion(RelativeRotation);
}

void UCameraComponent::Input()
{
    return;
	if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) // VK_RBUTTON은 마우스 오른쪽 버튼을 나타냄
	{
		if (!bRightMouseDown)
		{
			// 마우스 오른쪽 버튼을 처음 눌렀을 때, 마우스 위치 초기화
			bRightMouseDown = true;
			GetCursorPos(&lastMousePos);
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
			RotateYaw(deltaX * 0.1f);  // X 이동에 따라 좌우 회전
			RotatePitch(deltaY * 0.1f);  // Y 이동에 따라 상하 회전

			// 새로운 마우스 위치 저장
			lastMousePos = currentMousePos;
		}
		if (GetAsyncKeyState('A') & 0x8000)
		{
			MoveRight(-1.f);
		}
		if (GetAsyncKeyState('D') & 0x8000)
		{
			MoveRight(1.f);
		}
		if (GetAsyncKeyState('W') & 0x8000)
		{
			MoveForward(1.f);
		}
		if (GetAsyncKeyState('S') & 0x8000)
		{
			MoveForward(-1.f);
		}
		if (GetAsyncKeyState('E') & 0x8000)
		{
			MoveUp(1.f);
		}
		if (GetAsyncKeyState('Q') & 0x8000)
		{
			MoveUp(-1.f);
		}
	}
	else
	{
		bRightMouseDown = false; // 마우스 오른쪽 버튼을 떼면 상태 초기화
	}

}

void UCameraComponent::MoveForward(float _Value)
{
	RelativeLocation = RelativeLocation + GetForwardVector() * GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetCameraSpeedScalar() * _Value;
}

void UCameraComponent::MoveRight(float _Value)
{
	//FVector newRight = FVector(GetRightVector().X, GetRightVector().Y, 0.0f);
	RelativeLocation = RelativeLocation + GetRightVector() * GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetCameraSpeedScalar() * _Value;
}

void UCameraComponent::MoveUp(float _Value)
{
	RelativeLocation.Z += _Value * GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetCameraSpeedScalar();
}

void UCameraComponent::RotateYaw(float _Value)
{
	RelativeRotation.Z += _Value * GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetCameraSpeedScalar();
}

void UCameraComponent::RotatePitch(float _Value)
{

	RelativeRotation.Y += _Value;
	if (RelativeRotation.Y < -90.0f)
		RelativeRotation.Y = -90.0f;
	if (RelativeRotation.Y > 90.0f)
		RelativeRotation.Y = 90.0f;
}

FFrustum UCameraComponent::CreateFrustumFromCamera()
{
    FFrustum Frustum;

    FVector CamPos = GetWorldLocation();
    FVector Forward = zAxis.GetSafeNormal(); // 카메라 정면 방향 (+Z)
    FVector Right = xAxis.GetSafeNormal();   // 카메라 우측 방향 (+X)
    FVector Up = yAxis.GetSafeNormal();      // 카메라 상측 방향 (+Y)

    float AspectRatio = 16.0f / 9.0f; // 가정된 기본 종횡비

    float HalfV = tanf(FMath::DegreesToRadians(FOV) * 0.5f) * nearClip;
    float HalfH = HalfV * AspectRatio;

    FVector NearCenter = CamPos + Forward * nearClip;
    FVector FarCenter = CamPos + Forward * farClip;

    // Near / Far 평면
    FVector Back = Forward * -1.0f;  
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
    Frustum.Planes[2] = FPlane(TopNormal.Cross(Right), CamPos);

    // Bottom 평면
    FVector BottomNormal = (NearCenter - Up * HalfV - CamPos).GetSafeNormal();
    Frustum.Planes[3] = FPlane(Right.Cross(BottomNormal), CamPos);

    return Frustum;
}
