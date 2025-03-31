#include "Player.h"

#include "UnrealClient.h"
#include "WindowsPlatformTime.h"
#include "World.h"
#include "BaseGizmos/GizmoArrowComponent.h"
#include "BaseGizmos/GizmoCircleComponent.h"
#include "BaseGizmos/GizmoRectangleComponent.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "Components/LightComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "Math/JungleMath.h"
#include "Math/MathUtility.h"
#include "PropertyEditor/ShowFlags.h"
#include "Stats/Stats.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/UObjectIterator.h"
#include "GeometryCore/Octree.h"
#include "Editor/UnrealEd/PrimitiveBatch.h"
#include "GeometryCore/KDTree.h"
#include "GeometryCore/BVHNode.h"

#include "GeometryCore/SoftwareZBuffer.h"

using namespace DirectX;

// Picking 성능 측정 저장용 static 구조체
AEditorPlayer::FPickingTimeInfo AEditorPlayer::PickingTimeInfo{};

void AEditorPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    Input();
    // Octree없이 프러스텀 컬링 주석.
    //UpdateVisibleStaticMeshComponents();
    UpdateVisibleStaticMeshComponentsWithOctree();
}

void AEditorPlayer::Input()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {
        if (!bLeftMouseDown)
        {
            QUICK_SCOPE_CYCLE_COUNTER(PickingTime);
            bLeftMouseDown = true;

            POINT mousePos;
            GetCursorPos(&mousePos);
            GetCursorPos(&m_LastMousePos);

			// 컬러 피킹 부분입니다
//#if _DEBUG
            //uint32 UUID = GetEngine().graphicDevice.GetPixelUUID(mousePos);
            //// TArray<UObject*> objectArr = GetWorld()->GetObjectArr();
            //for ( const auto obj : TObjectRange<USceneComponent>())
            //{
            //    if (obj->GetUUID() != UUID) continue;

            //    UE_LOG(LogLevel::Display, "%s Pixel Pick", *obj->GetName());
            //}
//#endif
            ScreenToClient(GetEngine().hWnd, &mousePos);

            FVector pickPosition;

            const auto& ActiveViewport = GetEngine().GetLevelEditor()->GetActiveViewportClient();
            ScreenToViewSpace(mousePos.x, mousePos.y, ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix(), pickPosition);
            //bool res = PickGizmo(pickPosition);   
            //if (!res) PickActor(pickPosition);
            /*if (!res) */PickActorBVH(pickPosition);

            PickingTimeInfo.LastPickingTime.store(
                static_cast<float>(CycleCount_PickingTime.Finish())
                * FPlatformTime::GetSecondsPerCycle() *1000.0f,
                std::memory_order_relaxed
            );
            PickingTimeInfo.NumAttempts.fetch_add(1, std::memory_order_relaxed);
            PickingTimeInfo.AccumulatedTime.fetch_add(PickingTimeInfo.LastPickingTime, std::memory_order_relaxed);
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
        if (AActor* PickedActor = World->GetSelectedActor())
        {
            World->DestroyActor(PickedActor);
            World->SetPickedActor(nullptr);
        }
    }
}

bool AEditorPlayer::PickGizmo(const FVector& rayOrigin)
{
    bool isPickedGizmo = false;
    if (GetWorld()->GetSelectedActor())             // 현재 선택된 액터가 있다면
    {
        if (cMode == CM_TRANSLATION)
        {
            for (auto iter : GetWorld()->LocalGizmo->GetArrowArr()) // 3개의 ArrowArr 순회
            {
                int maxIntersect = 0;
                float minDistance = FLT_MAX;
                float Distance = 0.0f;
                int currentIntersectCount = 0;
                if (!iter) continue;
                if (RayIntersectsObject(rayOrigin, iter, Distance, currentIntersectCount))
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
            for (auto iter : GetWorld()->LocalGizmo->GetDiscArr())          // 3개의 rotation 디스크 모양 교차 검사
            {
                int maxIntersect = 0;
                float minDistance = FLT_MAX;
                float Distance = 0.0f;
                int currentIntersectCount = 0;
                //UPrimitiveComponent* localGizmo = dynamic_cast<UPrimitiveComponent*>(GetWorld()->LocalGizmo[i]);
                if (!iter) continue;
                if (RayIntersectsObject(rayOrigin, iter, Distance, currentIntersectCount))
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
                if (RayIntersectsObject(rayOrigin, iter, Distance, currentIntersectCount))
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

void AEditorPlayer::PickActor(const FVector& pickPosition)
{
    if (!(ShowFlags::GetInstance().currentFlags & EEngineShowFlags::SF_Primitives)) return;

    const UActorComponent* Possible = nullptr;
    int maxIntersect = 0;
    float minDistance = FLT_MAX;

    // 옥트리 시스템 가져오기
    UWorld* World = GetWorld();
	BVHSystem* BVH = World->GetBVHSystem();
    Ray MyRay = GetRayDirection(pickPosition);

#pragma region BVH Ray Intersection
    TArray<UPrimitiveComponent*> BVHComponents;
    BVH->Root->QueryRay(MyRay.Origin, MyRay.Direction, BVHComponents);

#pragma endregion
    for (const auto& iter : BVHComponents)
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
    else 
    {
        GetWorld()->SetPickedActor(nullptr);
    }
}

void AEditorPlayer::PickActorBVH(const FVector& pickPosition)
{
    const UActorComponent* Possible = nullptr;
    float minDistance = FLT_MAX;

    // 옥트리 시스템 가져오기
    UWorld* World = GetWorld();
    BVHSystem* BVH = World->GetBVHSystem();
    KDTreeSystem* KDTree = World->GetKDTreeSystem();
    Ray MyRay = GetRayDirection(pickPosition);


#pragma region BVH Ray Intersection
    float dist = FLT_MAX;
    //Possible = BVH->Root->QueryRayClosest(MyRay.Origin, MyRay.Direction);
    Possible = BVH ? BVH->Root->QueryRayClosestBestFirst(MyRay.Origin, MyRay.Direction) : nullptr;
    //Possible = KDTree->Root->QueryRayClosestBestFirst(MyRay.Origin, MyRay.Direction);
    //Possible = BVH->Root->QueryRayClosestSegmentTree(MyRay.Origin, MyRay.Direction);

    //TArray<UPrimitiveComponent*> BVHComps;
    //BVH->Root->QueryRaySorted(MyRay.Origin, MyRay.Direction, BVHComps);
    //Possible = BVHComps[0];
#pragma endregion

    if (Possible)
    {
        GetWorld()->SetPickedActor(Possible->GetOwner());
    }
    else
    {
        GetWorld()->SetPickedActor(nullptr);
    }
}

void AEditorPlayer::AddControlMode()
{
    cMode = static_cast<ControlMode>((cMode + 1) % CM_END);
}

void AEditorPlayer::AddCoordiMode()
{
    cdMode = static_cast<CoordiMode>((cdMode + 1) % CDM_END);
}

void AEditorPlayer::ScreenToViewSpace(int screenX, int screenY, const FMatrix& viewMatrix, const FMatrix& projectionMatrix, FVector& pickPosition)
{
    D3D11_VIEWPORT viewport = GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetD3DViewport();
    
    float viewportX = screenX - viewport.TopLeftX;
    float viewportY = screenY - viewport.TopLeftY;

    pickPosition.X = ((2.0f * viewportX / viewport.Width) - 1) / projectionMatrix[0][0];
    pickPosition.Y = -((2.0f * viewportY / viewport.Height) - 1) / projectionMatrix[1][1];
    if (GetEngine().GetLevelEditor()->GetActiveViewportClient()->IsOrtho())
    {
        pickPosition.Z = 0.0f;  // 오쏘 모드에서는 unproject 시 near plane 위치를 기준
    }
    else
    {
        pickPosition.Z = 1.0f;  // 퍼스펙티브 모드: near plane
    }
}

int AEditorPlayer::RayIntersectsObject(const FVector& pickPosition, USceneComponent* obj, float& hitDistance, int& intersectCount)
{
	FMatrix scaleMatrix = FMatrix::CreateScale(
		obj->GetWorldScale().X,
		obj->GetWorldScale().Y,
		obj->GetWorldScale().Z
	);
	FMatrix rotationMatrix = FMatrix::CreateRotation(
		obj->GetWorldRotation().X,
		obj->GetWorldRotation().Y,
		obj->GetWorldRotation().Z
	);

	FMatrix translationMatrix = FMatrix::CreateTranslationMatrix(obj->GetWorldLocation());

	FMatrix worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;
	FMatrix viewMatrix = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix();
    
    bool bIsOrtho = GetEngine().GetLevelEditor()->GetActiveViewportClient()->IsOrtho();
    

    if (bIsOrtho)
    {
        // 오쏘 모드: ScreenToViewSpace()에서 계산된 pickPosition이 클립/뷰 좌표라고 가정
        FMatrix inverseView = FMatrix::Inverse(viewMatrix);
        // pickPosition을 월드 좌표로 변환
        FVector worldPickPos = inverseView.TransformPosition(pickPosition);  
        // 오쏘에서는 픽킹 원점은 unproject된 픽셀의 위치
        FVector rayOrigin = worldPickPos;
        // 레이 방향은 카메라의 정면 방향 (평행)
        FVector orthoRayDir = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->ViewTransformOrthographic.GetForwardVector().GetSafeNormal();

        // 객체의 로컬 좌표계로 변환
        FMatrix localMatrix = FMatrix::Inverse(worldMatrix);
        FVector localRayOrigin = localMatrix.TransformPosition(rayOrigin);
        FVector localRayDir = (localMatrix.TransformPosition(rayOrigin + orthoRayDir) - localRayOrigin).GetSafeNormal();
        
        intersectCount = obj->CheckRayIntersection(localRayOrigin, localRayDir, hitDistance);
        return intersectCount;
    }
    else
    {
        FMatrix inverseMatrix = FMatrix::Inverse(worldMatrix * viewMatrix);
        FVector cameraOrigin = { 0,0,0 };
        FVector pickRayOrigin = inverseMatrix.TransformPosition(cameraOrigin);
        // 퍼스펙티브 모드의 기존 로직 사용
        FVector transformedPick = inverseMatrix.TransformPosition(pickPosition);
        FVector rayDirection = (transformedPick - pickRayOrigin).GetSafeNormal();
        
        intersectCount = obj->CheckRayIntersection(pickRayOrigin, rayDirection, hitDistance);
        return intersectCount;
    }
}

void AEditorPlayer::PickedObjControl()
{
    if (GetWorld()->GetSelectedActor() && GetWorld()->GetPickingGizmo())
    {
        POINT currentMousePos;
        GetCursorPos(&currentMousePos);
        int32 deltaX = currentMousePos.x - m_LastMousePos.x;
        int32 deltaY = currentMousePos.y - m_LastMousePos.y;

        // USceneComponent* pObj = GetWorld()->GetPickingObj();
        AActor* PickedActor = GetWorld()->GetSelectedActor();
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
        m_LastMousePos = currentMousePos;
    }
}

void AEditorPlayer::ControlRotation(USceneComponent* pObj, const UGizmoBaseComponent* Gizmo, int32 deltaX, int32 deltaY)
{
    const auto ActiveViewport = GetEngine().GetLevelEditor()->GetActiveViewportClient();
    const FViewportCameraTransform* ViewTransform = ActiveViewport->GetViewportType() == LVT_Perspective
                                                        ? &ActiveViewport->ViewTransformPerspective
                                                        : &ActiveViewport->ViewTransformOrthographic;

    FVector CameraForward = ViewTransform->GetForwardVector();
    FVector CameraRight = ViewTransform->GetRightVector();
    FVector CameraUp = ViewTransform->GetUpVector();

    FQuat currentRotation = pObj->GetQuat();
    
    FQuat rotationDelta;
    
    if (Gizmo->GetGizmoType() == UGizmoBaseComponent::CircleX)
    {
        float rotationAmount = (CameraUp.Z >= 0 ? -1.0f : 1.0f) * deltaY * 0.01f;
        rotationAmount = rotationAmount + (CameraRight.X >= 0 ? 1.0f : -1.0f) * deltaX * 0.01f;
    
        rotationDelta = FQuat(FVector(1.0f, 0.0f, 0.0f), rotationAmount); // ���� X �� ���� ȸ��
    }
    else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::CircleY)
    {
        float rotationAmount = (CameraRight.X >= 0 ? 1.0f : -1.0f) * deltaX * 0.01f;
        rotationAmount = rotationAmount + (CameraUp.Z >= 0 ? 1.0f : -1.0f) * deltaY * 0.01f;
    
        rotationDelta = FQuat(FVector(0.0f, 1.0f, 0.0f), rotationAmount); // ���� Y �� ���� ȸ��
    }
    else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::CircleZ)
    {
        float rotationAmount = (CameraForward.X <= 0 ? -1.0f : 1.0f) * deltaX * 0.01f;
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

void AEditorPlayer::ControlTranslation(USceneComponent* pObj, const UGizmoBaseComponent* Gizmo, int32 deltaX, int32 deltaY)
{
    float DeltaX = static_cast<float>(deltaX);
    float DeltaY = static_cast<float>(deltaY);
    const auto ActiveViewport = GetEngine().GetLevelEditor()->GetActiveViewportClient();
    const FViewportCameraTransform* ViewTransform = ActiveViewport->GetViewportType() == LVT_Perspective
                                                        ? &ActiveViewport->ViewTransformPerspective
                                                        : &ActiveViewport->ViewTransformOrthographic;

    FVector CameraRight = ViewTransform->GetRightVector();
    FVector CameraUp = ViewTransform->GetUpVector();
    
    FVector WorldMoveDirection = (CameraRight * DeltaX + CameraUp * -DeltaY) * 0.1f;
    
    if (cdMode == CDM_LOCAL)
    {
        if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowX)
        {
            float moveAmount = WorldMoveDirection.Dot(pObj->GetForwardVector());
            pObj->AddLocation(pObj->GetForwardVector() * moveAmount);
        }
        else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowY)
        {
            float moveAmount = WorldMoveDirection.Dot(pObj->GetRightVector());
            pObj->AddLocation(pObj->GetRightVector() * moveAmount);
        }
        else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowZ)
        {
            float moveAmount = WorldMoveDirection.Dot(pObj->GetUpVector());
            pObj->AddLocation(pObj->GetUpVector() * moveAmount);
        }
    }
    else if (cdMode == CDM_WORLD)
    {
        // 월드 좌표계에서 카메라 방향을 고려한 이동
        if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowX)
        {
            // 카메라의 오른쪽 방향을 X축 이동에 사용
            FVector moveDir = CameraRight * DeltaX * 0.05f;
            pObj->AddLocation(FVector(moveDir.X, 0.0f, 0.0f));
        }
        else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowY)
        {
            // 카메라의 오른쪽 방향을 Y축 이동에 사용
            FVector moveDir = CameraRight * DeltaX * 0.05f;
            pObj->AddLocation(FVector(0.0f, moveDir.Y, 0.0f));
        }
        else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowZ)
        {
            // 카메라의 위쪽 방향을 Z축 이동에 사용
            FVector moveDir = CameraUp * -DeltaY * 0.05f;
            pObj->AddLocation(FVector(0.0f, 0.0f, moveDir.Z));
        }
    }

    // 변화가 있을 때 pObj의 바운딩 박스 위치 업데이트
    UWorld* World = GetWorld();
    OctreeSystem* Octree = World->GetOctreeSystem();

    // ------------------------------------ // 
}

void AEditorPlayer::ControlScale(USceneComponent* pObj, const UGizmoBaseComponent* Gizmo, int32 deltaX, int32 deltaY)
{
    float DeltaX = static_cast<float>(deltaX);
    float DeltaY = static_cast<float>(deltaY);
    const auto ActiveViewport = GetEngine().GetLevelEditor()->GetActiveViewportClient();
    const FViewportCameraTransform* ViewTransform = ActiveViewport->GetViewportType() == LVT_Perspective
                                                        ? &ActiveViewport->ViewTransformPerspective
                                                        : &ActiveViewport->ViewTransformOrthographic;

    FVector CameraRight = ViewTransform->GetRightVector();
    FVector CameraUp = ViewTransform->GetUpVector();

    // 월드 좌표계에서 카메라 방향을 고려한 이동
    if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ScaleX)
    {
        // 카메라의 오른쪽 방향을 X축 이동에 사용
        FVector moveDir = CameraRight * DeltaX * 0.05f;
        pObj->AddScale(FVector(moveDir.X, 0.0f, 0.0f));
    }
    else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ScaleY)
    {
        // 카메라의 오른쪽 방향을 Y축 이동에 사용
        FVector moveDir = CameraRight * DeltaX * 0.05f;
        pObj->AddScale(FVector(0.0f, moveDir.Y, 0.0f));
    }
    else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ScaleZ)
    {
        // 카메라의 위쪽 방향을 Z축 이동에 사용
        FVector moveDir = CameraUp * -DeltaY * 0.05f;
        pObj->AddScale(FVector(0.0f, 0.0f, moveDir.Z));
    }
}

void AEditorPlayer::UpdateVisibleStaticMeshComponentsWithOctree()
{
    UWorld* World = GetWorld();
    FRenderer* Renderer = &FEngineLoop::renderer;
    OctreeSystem* Octree = World->GetOctreeSystem();
    if (!Octree || !Octree->Root) return;

    FFrustum Frustum = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->CreateFrustumFromCamera();
   
    // 보여지는 오브젝트들 초기화.
    Renderer->GetVisibleObjs().Empty();
    TSet<UPrimitiveComponent*> FrustumComps;
    TSet<uint32> UniqueUUIDs;
    Octree->Root->QueryFrustumUnique(Frustum, FrustumComps, UniqueUUIDs);
    
    Renderer->SetVisibleObjs(FrustumComps);
}

void AEditorPlayer::UpdateVisibleStaticMeshComponents() {
    UWorld* World = GetWorld();
    FRenderer* Renderer = &FEngineLoop::renderer;

    FFrustum Frustum = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->CreateFrustumFromCamera();

    Renderer->GetVisibleObjs().Empty();
    TSet<UPrimitiveComponent*> FrustumComps;
    TSet<uint32> UniqueUUIDs;

    // TODO : TObjectRange<UPrimitveComponent> 캐싱해서 사용하기 성능 확인해보기
    for (const auto iter : TObjectRange<UPrimitiveComponent>()) {

        // AABB 교차 검사
        FMatrix Model = JungleMath::CreateModelMatrix(
            iter->GetWorldLocation(),
            iter->GetWorldRotation(),
            iter->GetWorldScale()
        );
        FBoundingBox WorldBBox = UPrimitiveBatch::GetWorldBoundingBox(
            iter->AABB, iter->GetWorldLocation(), Model
        );
        if (Frustum.Intersects(WorldBBox))
        {
            UniqueUUIDs.Add(iter->GetUUID());
            FrustumComps.Add(iter);
        }
    }
    Renderer->SetVisibleObjs(FrustumComps);
}

Ray AEditorPlayer::GetRayDirection(const FVector& pickPosition)
{
    Ray result;
    FMatrix viewMatrix = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix();
    bool bIsOrtho = GetEngine().GetLevelEditor()->GetActiveViewportClient()->IsOrtho();

    if (bIsOrtho)
    {
        FMatrix inverseView = FMatrix::Inverse(viewMatrix);
        result.Origin = inverseView.TransformPosition(pickPosition);
        result.Direction = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->ViewTransformOrthographic.GetForwardVector().GetSafeNormal();
    }
    else
    {
        FMatrix inverseViewMatrix = FMatrix::Inverse(viewMatrix);
        FVector cameraOrigin = { 0,0,0 };
        result.Origin = inverseViewMatrix.TransformPosition(cameraOrigin);
        FVector transformedPick = inverseViewMatrix.TransformPosition(pickPosition);
        result.Direction = (transformedPick - result.Origin).GetSafeNormal();
    }

    return result;


}
