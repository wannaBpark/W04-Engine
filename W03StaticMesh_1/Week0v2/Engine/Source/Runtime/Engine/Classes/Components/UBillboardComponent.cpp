#include "UBillboardComponent.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
#include "Components/Player.h"
#include "Engine/Source/Editor/PropertyEditor/ShowFlags.h"
#include "QuadTexture.h"
#include "Define.h"
#include <DirectXMath.h>

#include "World.h"
#include "Math/MathUtility.h"

UBillboardComponent::UBillboardComponent()
{
    SetType("Quad");
}

UBillboardComponent::~UBillboardComponent()
{
	if (vertexTextureBuffer)
	{
		vertexTextureBuffer->Release();
		vertexTextureBuffer = nullptr;
	}
	if (indexTextureBuffer)
	{
		indexTextureBuffer->Release();
		indexTextureBuffer = nullptr;
	}
}

void UBillboardComponent::Initialize()
{
    Super::Initialize();
	CreateQuadTextureVertexBuffer();
}



void UBillboardComponent::Update(double deltaTime)
{
    Super::Update(deltaTime);
}

void UBillboardComponent::Release()
{
}

void UBillboardComponent::Render()
{
	FEngineLoop::renderer.PrepareTextureShader();
	//FEngineLoop::renderer.UpdateSubUVConstant(0, 0);
	//FEngineLoop::renderer.PrepareSubUVConstant();

	FMatrix M = CreateBillboardMatrix();
	FMatrix VP = GetEngine().View * GetEngine().Projection;

	// 최종 MVP 행렬
	FMatrix MVP = M * VP;
	if (this == GetWorld()->GetPickingGizmo()) {
		FEngineLoop::renderer.UpdateConstant(MVP, 1.0f);
	}
	else
		FEngineLoop::renderer.UpdateConstant(MVP, 0.0f);
    FEngineLoop::renderer.UpdateUUIDConstantBuffer(EncodeUUID());

	if (ShowFlags::GetInstance().currentFlags & static_cast<uint64>(EEngineShowFlags::SF_BillboardText)) {

	FEngineLoop::renderer.RenderTexturePrimitive(vertexTextureBuffer,numVertices,
		indexTextureBuffer,numIndices,Texture->TextureSRV,Texture->SamplerState);
	}

	FEngineLoop::renderer.PrepareShader();
}


int UBillboardComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{

	/*
	FMatrix worldMatrix = CreateBillboardMatrix();
	FMatrix ViewMatrix = GEngineLoop.View;
	FMatrix inverseMatrix = FMatrix::Inverse(worldMatrix * ViewMatrix);

	FVector cameraOrigin = { 0,0,0 };
	FVector pickRayOrigin = inverseMatrix.TransformPosition(cameraOrigin);


	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(GEngineLoop.hWnd, &mousePos);

	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	FEngineLoop::graphicDevice.DeviceContext->RSGetViewports(&numViewports, &viewport);
	float screenWidth = viewport.Width;
	float screenHeight = viewport.Height;

	FVector pickPosition;
	int screenX = mousePos.x;
	int screenY = mousePos.y;
	FMatrix projectionMatrix = GetEngine().Projection;
	pickPosition.x = ((2.0f * screenX / viewport.Width) - 1) / projectionMatrix[0][0];
	pickPosition.y = -((2.0f * screenY / viewport.Height) - 1) / projectionMatrix[1][1];
	pickPosition.z = 1.0f; // Near Plane

	FVector _rayDirection = inverseMatrix.TransformPosition(pickPosition);
	_rayDirection = (_rayDirection - pickRayOrigin).Normalize(); // local 좌표축의 ray

	return Super::CheckRayIntersection(pickRayOrigin, _rayDirection, pfNearHitDistance);
	*/
	
	TArray<FVector> quad;
	for (int i = 0; i < 4; i++)
	{
		quad.Add(FVector(quadTextureVertices[i].x, 
			quadTextureVertices[i].y, quadTextureVertices[i].z));
	}
	return CheckPickingOnNDC(quad,pfNearHitDistance);

}


void UBillboardComponent::SetTexture(FWString _fileName)
{
	Texture = FEngineLoop::resourceMgr.GetTexture(_fileName);
}

void UBillboardComponent::SetUUIDParent(USceneComponent* _parent)
{
	m_parent = _parent;
}


FMatrix UBillboardComponent::CreateBillboardMatrix()
{
	FMatrix CameraView = GetEngine().View;

	CameraView.M[0][3] = 0.0f;
	CameraView.M[1][3] = 0.0f;
	CameraView.M[2][3] = 0.0f;


	CameraView.M[3][0] = 0.0f;
	CameraView.M[3][1] = 0.0f;
	CameraView.M[3][2] = 0.0f;
	CameraView.M[3][3] = 1.0f;


	CameraView.M[0][2] = -CameraView.M[0][2];
	CameraView.M[1][2] = -CameraView.M[1][2];
	CameraView.M[2][2] = -CameraView.M[2][2];
	FMatrix LookAtCamera = FMatrix::Transpose(CameraView);
	
	FVector worldLocation = RelativeLocation;
	if (m_parent) worldLocation = RelativeLocation + m_parent->GetWorldLocation();
	FVector worldScale = RelativeScale3D;
	FMatrix S = FMatrix::CreateScale(worldScale.x, worldScale.y, worldScale.z);
	FMatrix R = LookAtCamera;
	FMatrix T = FMatrix::CreateTranslationMatrix(worldLocation);
	FMatrix M = S * R * T;

	return M;
}

void UBillboardComponent::CreateQuadTextureVertexBuffer()
{
	numVertices = sizeof(quadTextureVertices) / sizeof(FVertexTexture);
	numIndices = sizeof(quadTextureInices) / sizeof(uint32);
	vertexTextureBuffer = FEngineLoop::renderer.CreateVertexBuffer(quadTextureVertices, sizeof(quadTextureVertices));
	indexTextureBuffer = FEngineLoop::renderer.CreateIndexBuffer(quadTextureInices, sizeof(quadTextureInices));

	if (!vertexTextureBuffer) {
		Console::GetInstance().AddLog(LogLevel::Warning, "Buffer Error");
	}
	if (!indexTextureBuffer) {
		Console::GetInstance().AddLog(LogLevel::Warning, "Buffer Error");
	}
}

bool UBillboardComponent::CheckPickingOnNDC(const TArray<FVector>& checkQuad, float& hitDistance)
{
	bool result = false;
	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(GEngineLoop.hWnd, &mousePos);

	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	FEngineLoop::graphicDevice.DeviceContext->RSGetViewports(&numViewports, &viewport);
	float screenWidth = viewport.Width;
	float screenHeight = viewport.Height;

	FVector pickPosition;
	int screenX = mousePos.x;
	int screenY = mousePos.y;
	FMatrix projectionMatrix = GetEngine().Projection;
	pickPosition.x = ((2.0f * screenX / viewport.Width) - 1);
	pickPosition.y = -((2.0f * screenY / viewport.Height) - 1);
	pickPosition.z = 1.0f; // Near Plane

	FMatrix M = CreateBillboardMatrix();
	FMatrix V = GEngineLoop.View;
	FMatrix P = projectionMatrix;
	FMatrix MVP = M * V * P;

	float minX = FLT_MAX;
	float maxX = FLT_MIN;
	float minY = FLT_MAX;
	float maxY = FLT_MIN;
	float avgZ = 0.0f;
	for (int i = 0; i < checkQuad.Num(); i++)
	{
		FVector4 v = FVector4(checkQuad[i].x, checkQuad[i].y, checkQuad[i].z, 1.0f);
		FVector4 clipPos = FMatrix::TransformVector(v, MVP);
		
		if (clipPos.a != 0)	clipPos = clipPos/clipPos.a;

		minX = FMath::Min(minX, clipPos.x);
		maxX = FMath::Max(maxX, clipPos.x);
		minY = FMath::Min(minY, clipPos.y);
		maxY = FMath::Max(maxY, clipPos.y);
		avgZ += clipPos.z;
	}

	avgZ /= checkQuad.Num();

	if (pickPosition.x >= minX && pickPosition.x <= maxX &&
		pickPosition.y >= minY && pickPosition.y <= maxY)
	{
		float A = P.M[2][2];  // Projection Matrix의 A값 (Z 변환 계수)
		float B = P.M[3][2];  // Projection Matrix의 B값 (Z 변환 계수)

		float z_view_pick = (pickPosition.z - B) / A; // 마우스 클릭 View 공간 Z
		float z_view_billboard = (avgZ - B) / A; // Billboard View 공간 Z

		hitDistance = 1000.0f;
		result = true;
	}

	return result;
}
