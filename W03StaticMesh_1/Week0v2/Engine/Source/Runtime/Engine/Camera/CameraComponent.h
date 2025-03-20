#pragma once
#include "Components/SceneComponent.h"

class UCameraComponent : public USceneComponent
{
    DECLARE_CLASS(UCameraComponent, USceneComponent)

public:
    UCameraComponent();
    virtual ~UCameraComponent() override;

    virtual void Initialize() override;
    virtual void Update(double deltaTime) override;
    virtual void Release() override;

    void Input();
    void MoveForward(float _Value);
    void MoveRight(float _Value);
    void MoveUp(float _Value);
    void RotateYaw(float _Value);
    void RotatePitch(float _Value);

    bool IsCameraMode() const { return bRightMouseDown; }

private:
    float mouseSpeed = 0.25f;
    POINT lastMousePos;
    bool bRightMouseDown = false;

    FVector zAxis; // DirectX는 LH이므로 -z가 아니라 +z 사용
    FVector xAxis;
    FVector yAxis;
    float fov;

public:
    void SetFov(float _fov) { fov = _fov; }
    float& GetFov() { return fov; }

    void SetMouseSpeed(float _Value) { mouseSpeed = _Value; }
    float GetMouseSpeed() const { return mouseSpeed; }
};
