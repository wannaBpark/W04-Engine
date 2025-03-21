#include "GraphicDevice.h"
#include <wchar.h>
void FGraphicsDevice::Initialize(HWND hWindow) {
    CreateDeviceAndSwapChain(hWindow);
    CreateFrameBuffer();
    CreateDepthStencilBuffer(hWindow);
    CreateDepthStencilState();
    CreateRasterizerState();
    CurrentRasterizer = RasterizerStateSOLID;
}
void FGraphicsDevice::CreateDeviceAndSwapChain(HWND hWindow) {
    // 지원하는 Direct3D 기능 레벨을 정의
    D3D_FEATURE_LEVEL featurelevels[] = { D3D_FEATURE_LEVEL_11_0 };

    // 스왑 체인 설정 구조체 초기화
    DXGI_SWAP_CHAIN_DESC swapchaindesc = {};
    swapchaindesc.BufferDesc.Width = 0; // 창 크기에 맞게 자동으로 설정
    swapchaindesc.BufferDesc.Height = 0; // 창 크기에 맞게 자동으로 설정
    swapchaindesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // 색상 포맷
    swapchaindesc.SampleDesc.Count = 1; // 멀티 샘플링 비활성화
    swapchaindesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 렌더 타겟으로 사용
    swapchaindesc.BufferCount = 2; // 더블 버퍼링
    swapchaindesc.OutputWindow = hWindow; // 렌더링할 창 핸들
    swapchaindesc.Windowed = TRUE; // 창 모드
    swapchaindesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 스왑 방식

    // 디바이스와 스왑 체인 생성
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
        featurelevels, ARRAYSIZE(featurelevels), D3D11_SDK_VERSION,
        &swapchaindesc, &SwapChain, &Device, nullptr, &DeviceContext);

    if (FAILED(hr)) {
        MessageBox(hWindow, L"CreateDeviceAndSwapChain failed!", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    // 스왑 체인 정보 가져오기 (이후에 사용을 위해)
    ViewportResize();
    // 뷰포트 정보 설정
    //ViewportInfo = { 0.0f, 0.0f, (float)swapchaindesc.BufferDesc.Width, (float)swapchaindesc.BufferDesc.Height, 0.0f, 1.0f };
}



void FGraphicsDevice::CreateDepthStencilBuffer(HWND hWindow) {


    RECT clientRect;
    GetClientRect(hWindow, &clientRect);
    UINT width = clientRect.right - clientRect.left;
    UINT height = clientRect.bottom - clientRect.top;

    // 깊이/스텐실 텍스처 생성
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory(&descDepth, sizeof(descDepth));
    descDepth.Width = width; // 텍스처 너비 설정
    descDepth.Height = height; // 텍스처 높이 설정
    descDepth.MipLevels = 1; // 미맵 레벨 수 (1로 설정하여 미맵 없음)
    descDepth.ArraySize = 1; // 텍스처 배열의 크기 (1로 단일 텍스처)
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 24비트 깊이와 8비트 스텐실을 위한 포맷
    descDepth.SampleDesc.Count = 1; // 멀티샘플링 설정 (1로 단일 샘플)
    descDepth.SampleDesc.Quality = 0; // 샘플 퀄리티 설정
    descDepth.Usage = D3D11_USAGE_DEFAULT; // 텍스처 사용 방식
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL; // 깊이 스텐실 뷰로 바인딩 설정
    descDepth.CPUAccessFlags = 0; // CPU 접근 방식 설정
    descDepth.MiscFlags = 0; // 기타 플래그 설정

    HRESULT hr = Device->CreateTexture2D(&descDepth, NULL, &DepthStencilBuffer);

    if (FAILED(hr)) {
        MessageBox(hWindow, L"Failed to create depth stencilBuffer!", L"Error", MB_ICONERROR | MB_OK);
        return;
    }


    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 깊이 스텐실 포맷
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; // 뷰 타입 설정 (2D 텍스처)
    descDSV.Texture2D.MipSlice = 0; // 사용할 미맵 슬라이스 설정

    hr = Device->CreateDepthStencilView(DepthStencilBuffer, // Depth stencil texture
        &descDSV, // Depth stencil desc
        &DepthStencilView);  // [out] Depth stencil view

    if (FAILED(hr)) {
        wchar_t errorMsg[256];
        swprintf_s(errorMsg, L"Failed to create depth stencil view! HRESULT: 0x%08X", hr);
        MessageBox(hWindow, errorMsg, L"Error", MB_ICONERROR | MB_OK);
        return;
    }
}

void FGraphicsDevice::CreateDepthStencilState()
{
    // DepthStencil 상태 설명 설정
    D3D11_DEPTH_STENCIL_DESC dsDesc;

    // Depth test parameters
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

    // Stencil test parameters
    dsDesc.StencilEnable = true;
    dsDesc.StencilReadMask = 0xFF;
    dsDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing
    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    //// DepthStencil 상태 생성
    HRESULT hr = Device->CreateDepthStencilState(&dsDesc, &DepthStencilState);
    if (FAILED(hr)) {
        // 오류 처리
        return;
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = FALSE;  // 깊이 테스트 유지
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;  // 깊이 버퍼에 쓰지 않음
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;  // 깊이 비교를 항상 통과
    Device->CreateDepthStencilState(&depthStencilDesc, &DepthStateDisable);

}

void FGraphicsDevice::CreateRasterizerState()
{
    D3D11_RASTERIZER_DESC rasterizerdesc = {};
    rasterizerdesc.FillMode = D3D11_FILL_SOLID;
    rasterizerdesc.CullMode = D3D11_CULL_BACK;
    Device->CreateRasterizerState(&rasterizerdesc, &RasterizerStateSOLID);

    rasterizerdesc.FillMode = D3D11_FILL_WIREFRAME;
    rasterizerdesc.CullMode = D3D11_CULL_BACK;
    Device->CreateRasterizerState(&rasterizerdesc, &RasterizerStateWIREFRAME);
}


void FGraphicsDevice::ReleaseDeviceAndSwapChain()
{
    if (DeviceContext)
    {
        DeviceContext->Flush(); // 남아있는 GPU 명령 실행
    }

    if (SwapChain)
    {
        SwapChain->Release();
        SwapChain = nullptr;
    }

    if (Device)
    {
        Device->Release();
        Device = nullptr;
    }

    if (DeviceContext)
    {
        DeviceContext->Release();
        DeviceContext = nullptr;
    }
}

void FGraphicsDevice::CreateFrameBuffer()
{
    // 스왑 체인으로부터 백 버퍼 텍스처 가져오기
    SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);

    // 렌더 타겟 뷰 생성
    D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {};
    framebufferRTVdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // 색상 포맷
    framebufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

    Device->CreateRenderTargetView(FrameBuffer, &framebufferRTVdesc, &FrameBufferRTV);
}

void FGraphicsDevice::ReleaseFrameBuffer()
{
    if (FrameBuffer)
    {
        FrameBuffer->Release();
        FrameBuffer = nullptr;
    }

    if (FrameBufferRTV)
    {
        FrameBufferRTV->Release();
        FrameBufferRTV = nullptr;
    }
}

void FGraphicsDevice::ReleaseRasterizerState()
{
    if (RasterizerStateSOLID)
    {
        RasterizerStateSOLID->Release();
        RasterizerStateSOLID = nullptr;
    }
    if (RasterizerStateWIREFRAME)
    {
        RasterizerStateWIREFRAME->Release();
        RasterizerStateWIREFRAME = nullptr;
    }
}

void FGraphicsDevice::ReleaseDepthStencilResources()
{
    if (DepthStencilView) {
        DepthStencilView->Release();
        DepthStencilView = nullptr;
    }

    // 깊이/스텐실 버퍼 해제
    if (DepthStencilBuffer) {
        DepthStencilBuffer->Release();
        DepthStencilBuffer = nullptr;
    }

    // 깊이/스텐실 상태 해제
    if (DepthStencilState) {
        DepthStencilState->Release();
        DepthStencilState = nullptr;
    }
    if (DepthStateDisable) {
        DepthStateDisable->Release();
        DepthStateDisable = nullptr;
    }
}

void FGraphicsDevice::Release() 
{
    ReleaseRasterizerState();
    DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    ReleaseFrameBuffer();
    ReleaseDepthStencilResources();
    ReleaseDeviceAndSwapChain();
}

void FGraphicsDevice::SwapBuffer() {
    SwapChain->Present(1, 0);
}
void FGraphicsDevice::Prepare()
{
    DeviceContext->ClearRenderTargetView(FrameBufferRTV, ClearColor); // 렌더 타겟 뷰에 저장된 이전 프레임 데이터를 삭제
    DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0); // 깊이 버퍼 초기화 추가

    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정

    //DeviceContext->RSSetViewports(1, &ViewportInfo); // GPU가 화면을 렌더링할 영역 설정
    DeviceContext->RSSetState(CurrentRasterizer); //레스터 라이저 상태 설정

    DeviceContext->OMSetDepthStencilState(DepthStencilState, 0);

    DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, DepthStencilView); // 렌더 타겟 설정(백버퍼를 가르킴)
    DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff); // 블렌뎅 상태 설정, 기본블렌딩 상태임
}

void FGraphicsDevice::Prepare(UINT viewportNumber)
{
    DeviceContext->ClearRenderTargetView(FrameBufferRTV, ClearColor); // 렌더 타겟 뷰에 저장된 이전 프레임 데이터를 삭제
    DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0); // 깊이 버퍼 초기화 추가

    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정

    DeviceContext->RSSetViewports(1, &Viewports[viewportNumber]); // GPU가 화면을 렌더링할 영역 설정
    DeviceContext->RSSetState(CurrentRasterizer); //레스터 라이저 상태 설정

    DeviceContext->OMSetDepthStencilState(DepthStencilState, 0);

    DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, DepthStencilView); // 렌더 타겟 설정(백버퍼를 가르킴)
    DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff); // 블렌뎅 상태 설정, 기본블렌딩 상태임
}

void FGraphicsDevice::OnResize(HWND hWindow) {
    DeviceContext->OMSetRenderTargets(0, 0, 0);
    
    FrameBufferRTV->Release();
    FrameBufferRTV = nullptr;
    if (DepthStencilView) {
        DepthStencilView->Release();
        DepthStencilView = nullptr;
    }

    ReleaseFrameBuffer();

    // 윈도우 크기 가져오기
    RECT clientRect;
    GetClientRect(hWindow, &clientRect);
    UINT width = clientRect.right - clientRect.left;
    UINT height = clientRect.bottom - clientRect.top;

    if (width == 0 || height == 0) {
        MessageBox(hWindow, L"Invalid width or height for ResizeBuffers!", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    // SwapChain 크기 조정
    HRESULT hr;
    hr = SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0);  // DXGI_FORMAT_B8G8R8A8_UNORM으로 시도
    if (FAILED(hr)) {
        MessageBox(hWindow, L"failed", L"ResizeBuffers failed ", MB_ICONERROR | MB_OK);
        return;
    }
    
    CreateFrameBuffer();
    CreateDepthStencilBuffer(hWindow);

    ViewportResize();
}

void FGraphicsDevice::ViewportResize()
{
    DXGI_SWAP_CHAIN_DESC swapchaindesc = {};
    SwapChain->GetDesc(&swapchaindesc);


    float width = (float)swapchaindesc.BufferDesc.Width;
    float height = (float)swapchaindesc.BufferDesc.Height;
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;

    Viewports[0].TopLeftX = 0.0f;
    Viewports[0].TopLeftY = 0.0f;
    Viewports[0].Width = halfWidth;
    Viewports[0].Height = halfHeight;
    Viewports[0].MinDepth = 0.0f;
    Viewports[0].MaxDepth = 1.0f;

    // 뷰포트 2: 화면 오른쪽 위
    Viewports[1].TopLeftX = halfWidth;
    Viewports[1].TopLeftY = 0.0f;
    Viewports[1].Width = halfWidth;
    Viewports[1].Height = halfHeight;
    Viewports[1].MinDepth = 0.0f;
    Viewports[1].MaxDepth = 1.0f;

    // 뷰포트 3: 화면 왼쪽 아래
    Viewports[2].TopLeftX = 0.0f;
    Viewports[2].TopLeftY = halfHeight;
    Viewports[2].Width = halfWidth;
    Viewports[2].Height = halfHeight;
    Viewports[2].MinDepth = 0.0f;
    Viewports[2].MaxDepth = 1.0f;

    // 뷰포트 4: 화면 오른쪽 아래
    Viewports[3].TopLeftX = halfWidth;
    Viewports[3].TopLeftY = halfHeight;
    Viewports[3].Width = halfWidth;
    Viewports[3].Height = halfHeight;
    Viewports[3].MinDepth = 0.0f;
    Viewports[3].MaxDepth = 1.0f;
}

void FGraphicsDevice::ChangeRasterizer(EViewModeIndex evi)
{
    switch (evi)
    {
    case EViewModeIndex::VMI_Wireframe:
        CurrentRasterizer = RasterizerStateWIREFRAME;
        break;
    case EViewModeIndex::VMI_Lit:
    case EViewModeIndex::VMI_Unlit:
        CurrentRasterizer = RasterizerStateSOLID;
        break;
    }
    
}

void FGraphicsDevice::ChangeDepthStencilState(ID3D11DepthStencilState* newDetptStencil)
{
    DeviceContext->OMSetDepthStencilState(newDetptStencil, 0);
}
