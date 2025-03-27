#pragma once


class FViewport;
class UWorld;
class FViewportClient
{
public:
    virtual ~FViewportClient() = default;

    // FViewport에서 발생하는 이벤트를 처리하는 가상 함수들
    //virtual void OnInput(const FInputEvent& Event) = 0;
    virtual void Draw(FViewport* Viewport) = 0;
    virtual UWorld* GetWorld() const { return NULL; }
    // FViewport에 대한 참조 (혹은 소유)
};
    