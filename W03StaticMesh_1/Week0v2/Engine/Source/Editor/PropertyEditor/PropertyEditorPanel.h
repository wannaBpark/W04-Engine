#pragma once
#include "Components/ActorComponent.h"
#include "ImGUI/imgui.h"
#include "UnrealEd/EditorPanel.h"

class PropertyEditorPanel : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;


private:
    
private:
    float Width = 0, Height = 0;
};
