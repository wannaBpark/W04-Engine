#include "UTextRenderComponent.h"

#include "World.h"
#include "Engine/Source/Editor/PropertyEditor/ShowFlags.h"
#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"

UTextRenderComponent::UTextRenderComponent()
{
    SetType(StaticClass()->GetName());
}

UTextRenderComponent::~UTextRenderComponent()
{
	if (vertexTextBuffer)
	{
		vertexTextBuffer->Release();
		vertexTextBuffer = nullptr;
	}
}

void UTextRenderComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UTextRenderComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);

    //FVector newCamera = GetWorld()->GetCamera()->GetForwardVector();
    //newCamera.Z = 0;
    //newCamera = newCamera.GetSafeNormal();
    //float tmp = FVector(1.0f, 0.0f, 0.0f).Dot(newCamera);
    //float rad = acosf(tmp);
    //float degree = JungleMath::RadToDeg(rad);
    //FVector vtmp = FVector(1.0f, 0.0f, 0.0f).Cross(GetWorld()->GetCamera()->GetForwardVector());
    //if (vtmp.Z < 0)
    //	degree *= -1;
    //RelativeRotation.Z = degree + 90;
}

void UTextRenderComponent::ClearText()
{
    vertexTextureArr.Empty();
}
void UTextRenderComponent::SetRowColumnCount(int _cellsPerRow, int _cellsPerColumn) 
{
    RowCount = _cellsPerRow;
    ColumnCount = _cellsPerColumn;
}

int UTextRenderComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
	if (!(ShowFlags::GetInstance().currentFlags & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))) {
		return 0;
	}
	for (int i = 0; i < vertexTextureArr.Num(); i++)
	{
		quad.Add(FVector(vertexTextureArr[i].x,
			vertexTextureArr[i].y, vertexTextureArr[i].z));
	}

	return CheckPickingOnNDC(quad,pfNearHitDistance);
}

bool UTextRenderComponent::CheckPickingOnNDC(const TArray<FVector>& checkQuad, float& hitDistance)
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
    FMatrix projectionMatrix = GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetProjectionMatrix();
    pickPosition.X = ((2.0f * screenX / viewport.Width) - 1);
    pickPosition.Y = -((2.0f * screenY / viewport.Height) - 1);
    pickPosition.Z = 1.0f; // Near Plane

    FMatrix M = CreateTextMatrix();
    FMatrix V = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix();;
    FMatrix P = projectionMatrix;
    FMatrix MVP = M * V * P;

    float minX = FLT_MAX;
    float maxX = FLT_MIN;
    float minY = FLT_MAX;
    float maxY = FLT_MIN;
    float avgZ = 0.0f;
    for (int i = 0; i < checkQuad.Num(); i++)
    {
        FVector4 v = FVector4(checkQuad[i].X, checkQuad[i].Y, checkQuad[i].Z, 1.0f);
        FVector4 clipPos = FMatrix::TransformVector(v, MVP);

        if (clipPos.W != 0)	clipPos = clipPos / clipPos.W;

        minX = FMath::Min(minX, clipPos.X);
        maxX = FMath::Max(maxX, clipPos.X);
        minY = FMath::Min(minY, clipPos.Y);
        maxY = FMath::Max(maxY, clipPos.Y);
        avgZ += clipPos.W;
    }

    avgZ /= checkQuad.Num();

    if (pickPosition.X >= minX && pickPosition.X <= maxX &&
        pickPosition.Y >= minY && pickPosition.Y <= maxY)
    {
        float A = P.M[2][2];  // Projection Matrix의 A값 (Z 변환 계수)
        float B = P.M[3][2];  // Projection Matrix의 B값 (Z 변환 계수)

        float z_view_pick = (pickPosition.Z - B) / A; // 마우스 클릭 View 공간 Z
        float z_view_billboard = (avgZ - B) / A; // Billboard View 공간 Z

        hitDistance = 1000.0f;
        result = true;
    }

    return result;
}

void UTextRenderComponent::SetTexture(FWString _fileName)
{
    Texture = FEngineLoop::resourceMgr.GetTexture(_fileName);
}


void UTextRenderComponent::SetText(FWString _text)
{
	text = _text;
	if (_text.empty())
	{
		Console::GetInstance().AddLog(LogLevel::Warning, "Text is empty");

		vertexTextureArr.Empty();
		quad.Empty();

		// 기존 버텍스 버퍼가 있다면 해제
		if (vertexTextBuffer)
		{
			vertexTextBuffer->Release();
			vertexTextBuffer = nullptr;
		}
		return;
	}
	int textSize = static_cast<int>(_text.size());


	uint32 BitmapWidth = Texture->width;
	uint32 BitmapHeight = Texture->height;

	float CellWidth =  float(BitmapWidth)/ColumnCount;
	float CellHeight = float(BitmapHeight)/RowCount;

	float nTexelUOffset = CellWidth / BitmapWidth;
	float nTexelVOffset = CellHeight/ BitmapHeight;

	for (int i = 0; i < _text.size(); i++)
	{
        if (text[i] == '\0') {
            continue;
        }
		FVertexTexture leftUP = { -1.0f,1.0f,0.0f,0.0f,0.0f };
		FVertexTexture rightUP = { 1.0f,1.0f,0.0f,1.0f,0.0f };
		FVertexTexture leftDown = { -1.0f,-1.0f,0.0f,0.0f,1.0f };
		FVertexTexture rightDown = { 1.0f,-1.0f,0.0f,1.0f,1.0f };
		rightUP.u *= nTexelUOffset;
		leftDown.v *= nTexelVOffset;
		rightDown.u *= nTexelUOffset;
		rightDown.v *= nTexelVOffset;

		leftUP.x += quadWidth * i;
		rightUP.x += quadWidth * i;
		leftDown.x += quadWidth * i;
		rightDown.x += quadWidth * i;

		float startU = 0.0f;
		float startV = 0.0f;

		setStartUV(_text[i], startU, startV);
		leftUP.u += (nTexelUOffset * startU);
		leftUP.v += (nTexelVOffset * startV);
		rightUP.u += (nTexelUOffset * startU);
		rightUP.v += (nTexelVOffset * startV);
		leftDown.u += (nTexelUOffset * startU);
		leftDown.v += (nTexelVOffset * startV);
		rightDown.u += (nTexelUOffset * startU);
		rightDown.v += (nTexelVOffset * startV);

		vertexTextureArr.Add(leftUP);
		vertexTextureArr.Add(rightUP);
		vertexTextureArr.Add(leftDown);
		vertexTextureArr.Add(rightUP);
		vertexTextureArr.Add(rightDown);
		vertexTextureArr.Add(leftDown);
	}
	UINT byteWidth = static_cast<UINT>(vertexTextureArr.Num() * sizeof(FVertexTexture));

	float lastX = -1.0f + quadSize* _text.size();
	quad.Add(FVector(-1.0f,1.0f,0.0f));
	quad.Add(FVector(-1.0f,-1.0f,0.0f));
	quad.Add(FVector(lastX,1.0f,0.0f));
	quad.Add(FVector(lastX,-1.0f,0.0f));

	CreateTextTextureVertexBuffer(vertexTextureArr,byteWidth);
}
void UTextRenderComponent::setStartUV(wchar_t hangul, float& outStartU, float& outStartV)
{
    //대문자만 받는중
    int StartU = 0;
    int StartV = 0;
    int offset = -1;

    if (hangul == L' ') {
        outStartU = 0;  // Space는 특별히 UV 좌표를 (0,0)으로 설정
        outStartV = 0;
        offset = 0;
        return;
    }
    else if (hangul >= L'A' && hangul <= L'Z') {

        StartU = 11;
        StartV = 0;
        offset = hangul - L'A'; // 대문자 위치
    }
    else if (hangul >= L'a' && hangul <= L'z') {
        StartU = 37;
        StartV = 0;
        offset = (hangul - L'a'); // 소문자는 대문자 다음 위치
    }
    else if (hangul >= L'0' && hangul <= L'9') {
        StartU = 1;
        StartV = 0;
        offset = (hangul - L'0'); // 숫자는 소문자 다음 위치
    }
    else if (hangul >= L'가' && hangul <= L'힣')
    {
        StartU = 63;
        StartV = 0;
        offset = hangul - L'가'; // 대문자 위치
    }

    if (offset == -1)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "Text Error");
    }

    int offsetV = (offset + StartU) / ColumnCount;
    int offsetU = (offset + StartU) % ColumnCount;

    outStartU = static_cast<float>(offsetU);
    outStartV = static_cast<float>(StartV + offsetV);
}
void UTextRenderComponent::setStartUV(char alphabet, float& outStartU, float& outStartV)
{
    //대문자만 받는중
    int StartU=0;
    int StartV=0;
    int offset = -1;


    if (alphabet == ' ') {
        outStartU = 0;  // Space는 특별히 UV 좌표를 (0,0)으로 설정
        outStartV = 0;
        offset = 0;
        return;
    }
    else if (alphabet >= 'A' && alphabet <= 'Z') {

        StartU = 1;
        StartV = 4;
        offset = alphabet - 'A'; // 대문자 위치
    }
    else if (alphabet >= 'a' && alphabet <= 'z') {
        StartU = 1;
        StartV = 6;
        offset = (alphabet - 'a'); // 소문자는 대문자 다음 위치
    }
    else if (alphabet >= '0' && alphabet <= '9') {
        StartU = 0;
        StartV = 3;
        offset = (alphabet - '0'); // 숫자는 소문자 다음 위치
    }

    if (offset == -1)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "Text Error");
    }

    int offsetV = (offset + StartU) / ColumnCount;
    int offsetU = (offset + StartU) % ColumnCount;

    outStartU = static_cast<float>(offsetU);
    outStartV = static_cast<float>(StartV + offsetV);

}
void UTextRenderComponent::CreateTextTextureVertexBuffer(const TArray<FVertexTexture>& _vertex,UINT byteWidth)
{
	numTextVertices = static_cast<UINT>(_vertex.Num());
	// 2. Create a vertex buffer
	D3D11_BUFFER_DESC vertexbufferdesc = {};
	vertexbufferdesc.ByteWidth = byteWidth;
	vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
	vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexbufferSRD = { _vertex.GetData()};

	ID3D11Buffer* vertexBuffer;
	
	HRESULT hr = FEngineLoop::graphicDevice.Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);
	if (FAILED(hr))
	{
		UE_LOG(LogLevel::Warning, "VertexBuffer Creation faild");
	}
	vertexTextBuffer = vertexBuffer;

	//FEngineLoop::resourceMgr.RegisterMesh(&FEngineLoop::renderer, "JungleText", _vertex, _vertex.Num() * sizeof(FVertexTexture),
	//	nullptr, 0);

}

FMatrix UTextRenderComponent::CreateTextMatrix()
{
    FMatrix CameraView = GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix();

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
    FMatrix S = FMatrix::CreateScale(worldScale.X, worldScale.Y, worldScale.Z);
    FMatrix R = LookAtCamera;
    FMatrix T = FMatrix::CreateTranslationMatrix(worldLocation);
    FMatrix M = S * R * T;

    return M;
}


void UTextRenderComponent::TextMVPRendering()
{
    FEngineLoop::renderer.PrepareTextureShader();
    //FEngineLoop::renderer.UpdateSubUVConstant(0, 0);
    //FEngineLoop::renderer.PrepareSubUVConstant();
    FMatrix Model = CreateTextMatrix();

    FMatrix MVP = Model * GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix() * GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetProjectionMatrix();
    FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
    FVector4 UUIDColor = EncodeUUID() / 255.0f;
    if (this == GetWorld()->GetPickingGizmo()) {
        FEngineLoop::renderer.UpdateConstant(MVP, NormalMatrix, UUIDColor, true);
    }
    else
        FEngineLoop::renderer.UpdateConstant(MVP, NormalMatrix, UUIDColor, false);

    if (ShowFlags::GetInstance().currentFlags & static_cast<uint64>(EEngineShowFlags::SF_BillboardText)) {
        FEngineLoop::renderer.RenderTextPrimitive(vertexTextBuffer, numTextVertices,
            Texture->TextureSRV, Texture->SamplerState);
    }
    //Super::Render();

    FEngineLoop::renderer.PrepareShader();
}
