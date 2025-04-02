#pragma once
#include "Components/ActorComponent.h"
#include "UnrealEd/EditorPanel.h"
#include <functional>
#include <vector>

class UStaticMeshComponent;
class USceneComponent;

struct ComponentTypeInfo
{
    const char* Label;
    std::function<USceneComponent* (AActor*)> CreateFunc;
};

class PropertyEditorPanel : public IEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    void RGBToHSV(float r, float g, float b, float& h, float& s, float& v) const;
    void HSVToRGB(float h, float s, float v, float& r, float& g, float& b) const;

    /* Static Mesh Settings */
    void RenderForStaticMesh(UStaticMeshComponent* StaticMeshComp);

    /* Materials Settings */
    void RenderForMaterial(UStaticMeshComponent* StaticMeshComp);
    void RenderMaterialView(UMaterial* Material);
    void RenderCreateMaterialView();
    void DrawSceneComponentTree(USceneComponent* Comp);

private:
    float Width = 0, Height = 0;
    FVector Location = FVector(0, 0, 0);
    FVector Rotation = FVector(0, 0, 0);
    FVector Scale = FVector(0, 0, 0);

    /* Material Property */
    int SelectedMaterialIndex = -1;
    int CurMaterialIndex = -1;
    UStaticMeshComponent* SelectedStaticMeshComp = nullptr;
    FObjMaterialInfo tempMaterialInfo;
    bool IsCreateMaterial = false;

    USceneComponent* SelectedComponent = nullptr;

    int SelectedTypeIndex = 0;  // 초기 인덱스를 유효한 값으로 설정
};

// ComponentTypes 정의는 CPP 파일에 따로 둬야 함
extern std::vector<ComponentTypeInfo> ComponentTypes;