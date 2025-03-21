#pragma once
#include "Components/ActorComponent.h"
#include "UnrealEd/EditorPanel.h"

class ControlEditorPanel : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;


private:
    void CreateMenuButton(ImVec2 ButtonSize) const;
    void CreateFlagButton() const;
    void CreateSRTButton(ImVec2 ButtonSize) const;
    
private:
    float Width = 300, Height = 100;
};

