#pragma once
#include "Components/ActorComponent.h"
#include "ImGUI/imgui.h"
#include "UnrealEd/EditorPanel.h"

class UStaticMeshComponent;

class PropertyEditorPanel : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;


private:
    void RGBToHSV(float r, float g, float b, float& h, float& s, float& v);
    void HSVToRGB(float h, float s, float v, float& r, float& g, float& b);

    /* Materials Settings */
    void RenderForStaticMesh(UStaticMeshComponent* StaticMeshComp);
    void RenderMaterialView(UMaterial* Material);
    void RenderSubMeshView(UStaticMeshComponent* StaticMeshComp);
private:
    float Width = 0, Height = 0;
    FVector Location = FVector(0, 0, 0);
    FVector Rotation = FVector(0, 0, 0);
    FVector Scale = FVector(0, 0, 0);

    /* Material Property */
    uint32 SelectedMaterialIndex = -1;
    UStaticMeshComponent* SelectedStaticMeshComp = nullptr;
};
