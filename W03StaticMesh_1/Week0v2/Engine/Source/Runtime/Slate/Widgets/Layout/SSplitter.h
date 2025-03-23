#pragma once
#include "SlateCore/Widgets/SWindow.h"
class SSplitter : public SWindow
{
public:
    SWindow* SideLT; // Left or Top
    SWindow* SideRB; // Right or Bottom
    
    virtual void Initialize(FRect initRect);

    virtual void OnDragStart(const FPoint& mousePos) { /* 초기화 */ }
    virtual void OnDrag(const FPoint& delta) = 0; // 가로/세로에 따라 구현 다름.
    virtual void OnResize(float width, float height);

    virtual void OnDragEnd() {
        // 변경된 스플리터 위치 Editor.ini 저장 로직 구현
        SaveSplitterInfo();
    }

    void SaveSplitterInfo() {
        // Editor.ini에 현재 스플리터 상태(위치, 크기 등) 저장
    }

    // 스플리터가 포함된 영역에 따라 자식 창의 Rect를 재계산하는 함수
    virtual void UpdateChildRects() = 0;
};

class SSplitterH : public SSplitter
{
public:
    virtual void Initialize(FRect initRect);
    virtual void OnResize(float width, float height);
    void OnDrag(const FPoint& delta) override {
        // 수평 스플리터의 경우, 좌우로 이동
        Rect.leftTopX += delta.x;

        UpdateChildRects();
    }

    void UpdateChildRects() override {

        if (SideLT)
            SideLT->Rect.width = Rect.leftTopX - SideLT->Rect.leftTopX;
        if (SideRB)
        {
            float prevleftTopX = SideRB->Rect.leftTopX;
            SideRB->Rect.leftTopX = Rect.leftTopX + Rect.width;
            SideRB->Rect.width =  SideRB->Rect.width + prevleftTopX  - SideRB->Rect.leftTopX;
        }
    }
};

class SSplitterV : public SSplitter
{
public:
    virtual void Initialize(FRect initRect);
    virtual void OnResize(float width, float height);
    void OnDrag(const FPoint& delta) override {

        Rect.leftTopY += delta.y;
        UpdateChildRects();
    }
    void UpdateChildRects() override {

        if (SideLT)
            SideLT->Rect.height = Rect.leftTopY - SideLT->Rect.leftTopY;
        if (SideRB)
        {
            float prevleftTopY = SideRB->Rect.leftTopY;
            SideRB->Rect.leftTopY = Rect.leftTopY + Rect.height;
            SideRB->Rect.height = SideRB->Rect.height + prevleftTopY - SideRB->Rect.leftTopY;
        }
    }
};
