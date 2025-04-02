#include "UUIDRenderComponent.h"

UUIDRenderComponent::UUIDRenderComponent()
{
    SetScale(FVector(0.1f, 0.25f, 0.25f));
    SetLocation(FVector(0.0f, 0.0f, 5.0f));
}

//void UUIDRenderComponent::Render()
//{
//#pragma region GizmoDepth
//    ID3D11DepthStencilState* DepthStateDisable = FEngineLoop::graphicDevice.DepthStateDisable;
//    FEngineLoop::graphicDevice.DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
//#pragma endregion GizmoDepth
//
//    TextMVPRendering();
//
//#pragma region GizmoDepth
//    ID3D11DepthStencilState* originalDepthState = FEngineLoop::graphicDevice.DepthStencilState;
//    FEngineLoop::graphicDevice.DeviceContext->OMSetDepthStencilState(originalDepthState, 0);
//#pragma endregion GizmoDepth
//}

//void UUIDRenderComponent::BeginPlay()
//{
//    //SetText(std::to_wstring(GetOwner()->GetUUID()));
//    //SetTexture(L"Assets/Texture/font.png");
//    //SetRowColumnCount(106, 106);
//    //SetText(std::to_wstring(GetOwner()->GetUUID()));
//}


