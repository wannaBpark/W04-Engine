#include "Player.h"

#include "World.h"
#include "BaseGizmos/GizmoArrowComponent.h"
#include "BaseGizmos/GizmoCircleComponent.h"
#include "BaseGizmos/GizmoRectangleComponent.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "Components/LightComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "Math/MathUtility.h"
#include "PropertyEditor/ShowFlags.h"
#include "UnrealEd/EditorViewportClient.h"


using namespace DirectX;

UPlayer::UPlayer()
{
}

void UPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    Input();
}

void UPlayer::Input()
{
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {
        if (!bLeftMouseDown)
        {
            bLeftMouseDown = true;

            POINT mousePos;
            GetCursorPos(&mousePos);
            GetCursorPos(&m_LastMousePos);


            ScreenToClient(GEngineLoop.hWnd, &mousePos);

            FVector pickPosition;

            const auto& ActiveViewport = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();
            ScreenToViewSpace(mousePos.x, mousePos.y, ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix(), pickPosition);
            bool res = PickGizmo(pickPosition);
            if (!res) PickActor(pickPosition);
        }
        else
        {
            PickedObjControl();
        }
    }
    else
    {
        if (bLeftMouseDown)
        {
            bLeftMouseDown = false; // ���콺 ������ ��ư�� ���� ���� �ʱ�ȭ
            GetWorld()->SetPickingGizmo(nullptr);
        }
    }
    if (GetAsyncKeyState(VK_SPACE) & 0x8000)
    {
        if (!bSpaceDown)
        {
            AddControlMode();
            bSpaceDown = true;
        }
    }
    else
    {
        if (bSpaceDown)
        {
            bSpaceDown = false;
        }
    }
    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
    {
        if (!bRightMouseDown)
        {
            bRightMouseDown = true;
        }
    }
    else
    {
        bRightMouseDown = false;

        if (GetAsyncKeyState('Q') & 0x8000)
        {
            //GetWorld()->SetPickingObj(nullptr);
        }
        if (GetAsyncKeyState('W') & 0x8000)
        {
            cMode = CM_TRANSLATION;
        }
        if (GetAsyncKeyState('E') & 0x8000)
        {
            cMode = CM_ROTATION;
        }
        if (GetAsyncKeyState('R') & 0x8000)
        {
            cMode = CM_SCALE;
        }
    }

    if (GetAsyncKeyState(VK_DELETE) & 0x8000)
    {
        UWorld* World = GetWorld();
        if (AActor* PickedActor = World->GetPickedActor())
        {
            World->DestroyActor(PickedActor);
            World->SetPickedActor(nullptr);
        }
    }
}

bool UPlayer::PickGizmo(FVector& pickPosition)
{
    bool isPickedGizmo = false;
    if (GetWorld()->GetPickedActor())
    {
        if (cMode == CM_TRANSLATION)
        {
            for (auto iter : GetWorld()->LocalGizmo->GetArrowArr())
            {
                int maxIntersect = 0;
                float minDistance = FLT_MAX;
                float Distance = 0.0f;
                int currentIntersectCount = 0;
                if (!iter) continue;
                if (RayIntersectsObject(pickPosition, iter, Distance, currentIntersectCount))
                {
                    if (Distance < minDistance)
                    {
                        minDistance = Distance;
                        maxIntersect = currentIntersectCount;
                        GetWorld()->SetPickingGizmo(iter);
                        isPickedGizmo = true;
                    }
                    else if (abs(Distance - minDistance) < FLT_EPSILON && currentIntersectCount > maxIntersect)
                    {
                        maxIntersect = currentIntersectCount;
                        GetWorld()->SetPickingGizmo(iter);
                        isPickedGizmo = true;
                    }
                }
            }
        }
        else if (cMode == CM_ROTATION)
        {
            for (auto iter : GetWorld()->LocalGizmo->GetDiscArr())
            {
                int maxIntersect = 0;
                float minDistance = FLT_MAX;
                float Distance = 0.0f;
                int currentIntersectCount = 0;
                //UPrimitiveComponent* localGizmo = dynamic_cast<UPrimitiveComponent*>(GetWorld()->LocalGizmo[i]);
                if (!iter) continue;
                if (RayIntersectsObject(pickPosition, iter, Distance, currentIntersectCount))
                {
                    if (Distance < minDistance)
                    {
                        minDistance = Distance;
                        maxIntersect = currentIntersectCount;
                        GetWorld()->SetPickingGizmo(iter);
                        isPickedGizmo = true;
                    }
                    else if (abs(Distance - minDistance) < FLT_EPSILON && currentIntersectCount > maxIntersect)
                    {
                        maxIntersect = currentIntersectCount;
                        GetWorld()->SetPickingGizmo(iter);
                        isPickedGizmo = true;
                    }
                }
            }
        }
        else if (cMode == CM_SCALE)
        {
            for (auto iter : GetWorld()->LocalGizmo->GetScaleArr())
            {
                int maxIntersect = 0;
                float minDistance = FLT_MAX;
                float Distance = 0.0f;
                int currentIntersectCount = 0;
                if (!iter) continue;
                if (RayIntersectsObject(pickPosition, iter, Distance, currentIntersectCount))
                {
                    if (Distance < minDistance)
                    {
                        minDistance = Distance;
                        maxIntersect = currentIntersectCount;
                        GetWorld()->SetPickingGizmo(iter);
                        isPickedGizmo = true;
                    }
                    else if (abs(Distance - minDistance) < FLT_EPSILON && currentIntersectCount > maxIntersect)
                    {
                        maxIntersect = currentIntersectCount;
                        GetWorld()->SetPickingGizmo(iter);
                        isPickedGizmo = true;
                    }
                }
            }
        }
    }
    return isPickedGizmo;
}

void UPlayer::PickActor(const FVector& pickPosition)
{
    if (!(ShowFlags::GetInstance().currentFlags & EEngineShowFlags::SF_Primitives)) return;

    const UActorComponent* Possible = nullptr;
    int maxIntersect = 0;
    float minDistance = FLT_MAX;
    for (const auto iter : GUObjectArray.GetObjectItemArrayUnsafe()) // TODO: TObjectRange<PrimitiveComponent>로 사용
    {
        UPrimitiveComponent* pObj;
        if (iter->IsA<UPrimitiveComponent>() || iter->IsA<ULightComponentBase>())
        {
            pObj = static_cast<UPrimitiveComponent*>(iter);
        }
        else
        {
            continue;
        }

        if (pObj && !pObj->IsA<UGizmoBaseComponent>())
        {
            float Distance = 0.0f;
            int currentIntersectCount = 0;
            if (RayIntersectsObject(pickPosition, pObj, Distance, currentIntersectCount))
            {
                if (Distance < minDistance)
                {
                    minDistance = Distance;
                    maxIntersect = currentIntersectCount;
                    Possible = pObj;
                }
                else if (abs(Distance - minDistance) < FLT_EPSILON && currentIntersectCount > maxIntersect)
                {
                    maxIntersect = currentIntersectCount;
                    Possible = pObj;
                }
            }
        }
    }
    if (Possible)
    {
        GetWorld()->SetPickedActor(Possible->GetOwner());
    }
}

void UPlayer::AddControlMode()
{
    cMode = static_cast<ControlMode>((cMode + 1) % CM_END);
}

void UPlayer::AddCoordiMode()
{
    cdMode = static_cast<CoordiMode>((cdMode + 1) % CDM_END);
}

void UPlayer::ScreenToViewSpace(int screenX, int screenY, const FMatrix& viewMatrix, const FMatrix& projectionMatrix, FVector& pickPosition)
{
    D3D11_VIEWPORT viewport;
    UINT numViewports = 1;
    FEngineLoop::graphicDevice.DeviceContext->RSGetViewports(&numViewports, &viewport);
    float screenWidth = viewport.Width;
    float screenHeight = viewport.Height;

    pickPosition.x = ((2.0f * screenX / viewport.Width) - 1) / projectionMatrix[0][0];
    pickPosition.y = -((2.0f * screenY / viewport.Height) - 1) / projectionMatrix[1][1];
    pickPosition.z = 1.0f; // Near Plane
}

int UPlayer::RayIntersectsObject(const FVector& pickPosition, UPrimitiveComponent* obj, float& hitDistance, int& intersectCount)
{
    // ������Ʈ�� ���� ��ȯ ��� ���� (��ġ, ȸ��, ũ�� ����)
    FMatrix scaleMatrix = FMatrix::CreateScale(
        obj->GetWorldScale().x,
        obj->GetWorldScale().y,
        obj->GetWorldScale().z
    );

    //FMatrix rotationMatrix = JungleMath::CreateRotationMatrix(obj->GetWorldRotation());
    //FMatrix rotationMatrix = JungleMath::EulerToQuaternion(obj->GetWorldRotation()).ToMatrix();
    FMatrix rotationMatrix = FMatrix::CreateRotation(
        obj->GetWorldRotation().x,
        obj->GetWorldRotation().y,
        obj->GetWorldRotation().z
    );

    FMatrix translationMatrix = FMatrix::CreateTranslationMatrix(obj->GetWorldLocation());

    // ���� ��ȯ ���
    FMatrix worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

    FMatrix ViewMatrix = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix();
    FMatrix inverseMatrix = FMatrix::Inverse(worldMatrix * ViewMatrix);

    FVector cameraOrigin = {0, 0, 0};

    FVector pickRayOrigin = inverseMatrix.TransformPosition(cameraOrigin);
    FVector rayDirection = inverseMatrix.TransformPosition(pickPosition);
    rayDirection = (rayDirection - pickRayOrigin).Normalize(); // local ��ǥ���� ray
    intersectCount = obj->CheckRayIntersection(pickRayOrigin, rayDirection, hitDistance);

    return intersectCount;
}

void UPlayer::PickedObjControl()
{
    // ���콺 �̵��� ���
    if (GetWorld()->GetPickedActor() && GetWorld()->GetPickingGizmo())
    {
        POINT currentMousePos;
        GetCursorPos(&currentMousePos);
        // ���콺 �̵� ���� ���
        int32 deltaX = currentMousePos.x - m_LastMousePos.x;
        int32 deltaY = currentMousePos.y - m_LastMousePos.y;

        // USceneComponent* pObj = GetWorld()->GetPickingObj();
        AActor* PickedActor = GetWorld()->GetPickedActor();
        UGizmoBaseComponent* Gizmo = static_cast<UGizmoBaseComponent*>(GetWorld()->GetPickingGizmo());
        switch (cMode)
        {
        case CM_TRANSLATION:
            ControlTranslation(PickedActor->GetRootComponent(), Gizmo, deltaX, deltaY);
            break;
        case CM_SCALE:
            ControlScale(PickedActor->GetRootComponent(), Gizmo, deltaX, deltaY);

            break;
        case CM_ROTATION:
            ControlRotation(PickedActor->GetRootComponent(), Gizmo, deltaX, deltaY);
            break;
        default:
            break;
        }
        // ���ο� ���콺 ��ġ ����
        m_LastMousePos = currentMousePos;
    }
}

void UPlayer::ControlRotation(USceneComponent* pObj, UGizmoBaseComponent* Gizmo, int32 deltaX, int32 deltaY)
{
    FVector cameraForward = GetWorld()->GetCamera()->GetForwardVector();
    FVector cameraRight = GetWorld()->GetCamera()->GetRightVector();
    FVector cameraUp = GetWorld()->GetCamera()->GetUpVector();

    // ���� ���� ȸ���� ��������
    FQuat currentRotation = pObj->GetQuat();

    FQuat rotationDelta;

    if (Gizmo->GetType() == "CircleX")
    {
        float rotationAmount = (cameraUp.z >= 0 ? -1.0f : 1.0f) * deltaY * 0.01f;
        rotationAmount = rotationAmount + (cameraRight.x >= 0 ? 1.0f : -1.0f) * deltaX * 0.01f;

        rotationDelta = FQuat(FVector(1.0f, 0.0f, 0.0f), rotationAmount); // ���� X �� ���� ȸ��
    }
    else if (Gizmo->GetType() == "CircleY")
    {
        float rotationAmount = (cameraRight.x >= 0 ? 1.0f : -1.0f) * deltaX * 0.01f;
        rotationAmount = rotationAmount + (cameraUp.z >= 0 ? 1.0f : -1.0f) * deltaY * 0.01f;

        rotationDelta = FQuat(FVector(0.0f, 1.0f, 0.0f), rotationAmount); // ���� Y �� ���� ȸ��
    }
    else if (Gizmo->GetType() == "CircleZ")
    {
        float rotationAmount = (cameraForward.x <= 0 ? -1.0f : 1.0f) * deltaX * 0.01f;
        rotationDelta = FQuat(FVector(0.0f, 0.0f, 1.0f), rotationAmount); // ���� Z �� ���� ȸ��
    }
    if (cdMode == CDM_LOCAL)
    {
        pObj->SetRotation(currentRotation * rotationDelta);
    }
    else if (cdMode == CDM_WORLD)
    {
        pObj->SetRotation(rotationDelta * currentRotation);
    }
}

void UPlayer::ControlTranslation(USceneComponent* pObj, UGizmoBaseComponent* Gizmo, int32 deltaX, int32 deltaY)
{
    float deltaXf = static_cast<float>(deltaX);
    float deltaYf = static_cast<float>(deltaY);
    FVector vecObjToCamera = GetWorld()->GetCamera()->GetWorldLocation() - pObj->GetWorldLocation();
    FVector cameraRight = GetWorld()->GetCamera()->GetRightVector();
    FVector cameraUp = GetWorld()->GetCamera()->GetUpVector();
    FVector worldMoveDir = (cameraRight * deltaXf + cameraUp * -deltaYf) * 0.01f;

    if (cdMode == CDM_LOCAL)
    {
        if (Gizmo->GetType() == "ArrowX")
        {
            float moveAmount = worldMoveDir.Dot(pObj->GetForwardVector());
            pObj->AddLocation(pObj->GetForwardVector() * moveAmount);
        }
        else if (Gizmo->GetType() == "ArrowY")
        {
            float moveAmount = worldMoveDir.Dot(pObj->GetRightVector());
            pObj->AddLocation(pObj->GetRightVector() * moveAmount);
        }
        else if (Gizmo->GetType() == "ArrowZ")
        {
            float moveAmount = worldMoveDir.Dot(pObj->GetUpVector());
            pObj->AddLocation(pObj->GetUpVector() * moveAmount);
        }
    }
    else if (cdMode == CDM_WORLD)
    {
        if (Gizmo->GetType() == "ArrowX")
        {
            vecObjToCamera = FVector(vecObjToCamera.x, vecObjToCamera.y, pObj->GetLocalLocation().z);
            float dotResult = vecObjToCamera.Dot(FVector(1.0f, 0.0f, 0.0f));
            dotResult = dotResult / vecObjToCamera.Magnitude();
            float rad = acosf(dotResult);
            float degree = FMath::RadiansToDegrees(rad);
            FVector crossResult = vecObjToCamera.Cross(FVector(1.0f, 0.0f, 0.0f));
            if (crossResult.z > 0)
                degree *= -1.0f;
            //UE_LOG(LogLevel::Error, "%f", degree);

            if (0 < degree && degree < 180.0f)
                pObj->AddLocation(FVector(1.0f, 0.0f, 0.0f) * deltaXf * 0.01f);
            else if (degree < 0 && degree > -180.0f)
            {
                pObj->AddLocation(FVector(1.0f, 0.0f, 0.0f) * deltaXf * -0.01f);
            }
        }
        else if (Gizmo->GetType() == "ArrowY")
        {
            vecObjToCamera = FVector(vecObjToCamera.x, vecObjToCamera.y, pObj->GetLocalLocation().z);
            float dotResult = vecObjToCamera.Dot(FVector(0.0f, 1.0f, 0.0f));
            dotResult = dotResult / vecObjToCamera.Magnitude();
            float rad = acosf(dotResult);
            float degree = FMath::RadiansToDegrees(rad);
            FVector crossResult = vecObjToCamera.Cross(FVector(0.0f, 1.0f, 0.0f));
            if (crossResult.z > 0)
                degree *= -1.0f;
            //UE_LOG(LogLevel::Error, "%f", degree);
            if (0 < degree && degree < 180)
                pObj->AddLocation(FVector(0.0f, 1.0f, 0.0f) * deltaXf * 0.01f);
            else
                pObj->AddLocation(FVector(0.0f, 1.0f, 0.0f) * deltaXf * -0.01f);
        }
        else if (Gizmo->GetType() == "ArrowZ")
        {
            pObj->AddLocation(FVector(0.0f, 0.0f, 1.0f) * deltaYf * -0.01f);
        }
    }
}

void UPlayer::ControlScale(USceneComponent* pObj, UGizmoBaseComponent* Gizmo, int32 deltaX, int32 deltaY)
{
    FVector vecObjToCamera = GetWorld()->GetCamera()->GetWorldLocation() - pObj->GetWorldLocation();
    float deltaXf = static_cast<float>(deltaX);
    float deltaYf = static_cast<float>(deltaY);
    if (Gizmo->GetType() == "ScaleX")
    {
        vecObjToCamera = FVector(vecObjToCamera.x, vecObjToCamera.y, pObj->GetLocalLocation().z);
        float dotResult = vecObjToCamera.Dot(FVector(1.0f, 0.0f, 0.0f));
        dotResult = dotResult / vecObjToCamera.Magnitude();
        float rad = acosf(dotResult);
        float degree = FMath::RadiansToDegrees(rad);
        FVector crossResult = vecObjToCamera.Cross(FVector(1.0f, 0.0f, 0.0f));
        if (crossResult.z > 0)
            degree *= -1.0f;
        //UE_LOG(LogLevel::Error, "%f", degree);

        if (0 < degree && degree < 180.0f)
            pObj->AddScale(FVector(1.0f, 0.0f, 0.0f) * deltaXf * 0.01f);
        else if (degree < 0 && degree > -180.0f)
        {
            pObj->AddScale(FVector(1.0f, 0.0f, 0.0f) * deltaXf * -0.01f);
        }
    }
    else if (Gizmo->GetType() == "ScaleY")
    {
        vecObjToCamera = FVector(vecObjToCamera.x, vecObjToCamera.y, pObj->GetLocalLocation().z);
        float dotResult = vecObjToCamera.Dot(FVector(0.0f, 1.0f, 0.0f));
        dotResult = dotResult / vecObjToCamera.Magnitude();
        float rad = acosf(dotResult);
        float degree = FMath::RadiansToDegrees(rad);
        FVector crossResult = vecObjToCamera.Cross(FVector(0.0f, 1.0f, 0.0f));
        if (crossResult.z > 0)
            degree *= -1.0f;
        //UE_LOG(LogLevel::Error, "%f", degree);
        if (0 < degree && degree < 180)
            pObj->AddScale(FVector(0.0f, 1.0f, 0.0f) * deltaXf * 0.01f);
        else
            pObj->AddScale(FVector(0.0f, 1.0f, 0.0f) * deltaXf * -0.01f);
    }
    else if (Gizmo->GetType() == "ScaleZ")
    {
        pObj->AddScale(FVector(0.0f, 0.0f, 1.0f) * deltaYf * -0.01f);
    }
}
