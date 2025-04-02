#pragma once
#include "Components/ActorComponent.h"
#include "UnrealEd/EditorPanel.h"

class PlayEditorPanel : public IEditorPanel
{
public:
	PlayEditorPanel() = default;
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;


private:
    void CreatePlayButton(ImVec2 ButtonSize, ImFont* IconFont);
    void CreateStopButton(ImVec2 ButtonSize, ImFont* IconFont);
    void CreatePauseButton(ImVec2 ButtonSize, ImFont* IconFont) const;

    float Width = 300, Height = 100;
};
