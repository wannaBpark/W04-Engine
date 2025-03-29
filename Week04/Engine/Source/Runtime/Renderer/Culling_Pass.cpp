//#include "Culling_Pass.h"
//#include "EngineLoop.h"
//
//extern FEngineLoop gEngineLoop;
//
//void Culling_Pass::InitializeOcclusionCulling()
//{
//    
//    // HZB 텍스처 생성 – 전체 mip 체인 생성 (MipLevels = 0)
//    D3D11_TEXTURE2D_DESC hizDesc = {};
//    hizDesc.Width = 512;  // 고정 또는 설정값
//    hizDesc.Height = 256;
//    hizDesc.MipLevels = 0;
//    hizDesc.ArraySize = 1;
//    hizDesc.Format = DXGI_FORMAT_R32_FLOAT;
//    hizDesc.SampleDesc.Count = 1;
//    hizDesc.Usage = D3D11_USAGE_DEFAULT;
//    hizDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
//    hizDesc.CPUAccessFlags = 0;
//    hizDesc.MiscFlags = 0;
//    
//    gEngineLoop.graphicDevice.Device->CreateTexture2D(&hizDesc, nullptr, &m_HizBuffer);
//    m_HizBuffer->GetDesc(&hizDesc);
//    m_MaxMipLevel = hizDesc.MipLevels;
//    m_HizRTVs.resize(m_MaxMipLevel);
//    m_HizSRVs.resize(m_MaxMipLevel);
//    for (UINT miplevel = 0; miplevel < m_MaxMipLevel; miplevel++)
//    {
//        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
//        rtvDesc.Format = hizDesc.Format;
//        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
//        rtvDesc.Texture2D.MipSlice = miplevel;
//        gEngineLoop.graphicDevice.Device->CreateRenderTargetView(m_HizBuffer, &rtvDesc, &m_HizRTVs[miplevel]);
//
//        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
//        srvDesc.Format = hizDesc.Format;
//        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
//        srvDesc.Texture2D.MipLevels = 1;
//        srvDesc.Texture2D.MostDetailedMip = miplevel;
//        gEngineLoop.graphicDevice.Device->CreateShaderResourceView(m_HizBuffer, &srvDesc, &m_HizSRVs[miplevel]);
//    }
//    // 전체 mip 체인을 볼 수 있는 SRV
//    D3D11_SHADER_RESOURCE_VIEW_DESC fullSRVDesc = {};
//    fullSRVDesc.Format = hizDesc.Format;
//    fullSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
//    fullSRVDesc.Texture2D.MipLevels = hizDesc.MipLevels;
//    fullSRVDesc.Texture2D.MostDetailedMip = 0;
//    gEngineLoop.graphicDevice.Device->CreateShaderResourceView(m_HizBuffer, &fullSRVDesc, &m_HizSRV);
//
//    // HZB DepthStencilView 생성 (생성 API는 내부적으로 호출)
//    // 예를 들어 g_Factory->CreateDepthStencil<DS_HizDepth>(...)와 같이...
//    // m_HizDSV = ...;
//
//    // HZB 뷰포트 설정
//    m_HizViewport.TopLeftX = 0.0f;
//    m_HizViewport.TopLeftY = 0.0f;
//    m_HizViewport.Width = static_cast<float>(hizDesc.Width);
//    m_HizViewport.Height = static_cast<float>(hizDesc.Height);
//    m_HizViewport.MinDepth = 0.0f;
//    m_HizViewport.MaxDepth = 1.0f;
//
//    // Occlusion 컬링용 버퍼 생성
//    CreateOcclusionBuffers();
//}
//
//// Occluder 렌더링: HZB 최상위 mip에 occluder 메시들을 Depth 렌더링
//void Culling_Pass::RenderOccludersHZB()
//{
//    if (g_GlobalData->OccluderList.empty())
//        return;
//
//    CameraData* cam = g_GlobalData->MainCamera_Data;
//    FMatrix viewproj = cam->CamViewProj;
//
//    // HZB 렌더 타겟 및 뷰포트 설정
//    gEngineLoop.graphicDevice.DeviceContext->RSSetViewports(1, &m_HizViewport);
//    gEngineLoop.graphicDevice.DeviceContext->OMSetRenderTargets(1, &m_HizRTVs[0], m_HizDSV);
//    gEngineLoop.graphicDevice.DeviceContext->ClearRenderTargetView(m_HizRTVs[0], reinterpret_cast<const float*>(&DXColors::White));
//    gEngineLoop.graphicDevice.DeviceContext->ClearDepthStencilView(m_HizDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
//    gEngineLoop.graphicDevice.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//    // 각 occluder 메시 렌더링 (Depth 전용 셰이더 사용)
//    for (MeshBuffer* occluder : g_GlobalData->OccluderList)
//    {
//        // 오리지널 Vertex/Index Buffer 획득
//        ID3D11Buffer* originVertex = occluder->VertexBuf;
//        ID3D11Buffer* originIndex = occluder->IndexBuf;
//        ID3D11Buffer* convertVertex = (ID3D11Buffer*)originVertex->pVertexBuf;
//        ID3D11Buffer* convertIndex = (ID3D11Buffer*)originIndex->pIndexBuf;
//
//        // Depth 렌더링용 상수 버퍼 업데이트 (viewproj 등)
//        CB_DepthStaticMesh objectBuf;
//        objectBuf.gWorldViewProj = viewproj;
//        m_Depth_VS->ConstantBufferUpdate(&objectBuf);
//        m_Depth_VS->Update();
//        m_Depth_PS->Update();
//
//        gEngineLoop.graphicDevice.DeviceContext->IASetVertexBuffers(0, 1, &convertVertex, &originVertex->Stride, &originVertex->Offset);
//        gEngineLoop.graphicDevice.DeviceContext->IASetIndexBuffer(convertIndex, DXGI_FORMAT_R32_UINT, 0);
//        gEngineLoop.graphicDevice.DeviceContext->DrawIndexed(originIndex->Count, 0, 0);
//    }
//
//    // MipChain 생성 (별도 CommandList로 미리 예약한 경우)
//    gEngineLoop.graphicDevice.DeviceContext->ExecuteCommandList(m_MipCommandList, FALSE);
//}
//
//// Occlusion 컬링용 버퍼 생성 (Collider, Culling 결과 등)
//void Culling_Pass::CreateOcclusionBuffers()
//{
//    // 예: CullingRenderMeshList.size()를 기준으로
//    UINT renderCount = static_cast<UINT>(CullingRenderMeshList.size());
//    if (renderCount == 0)
//        return;
//
//    m_OcclusionResultList.resize(renderCount);
//
//    // Collider Buffer
//    D3D11_BUFFER_DESC colliderDesc = {};
//    colliderDesc.Usage = D3D11_USAGE_DYNAMIC;
//    colliderDesc.ByteWidth = sizeof(FVector4) * renderCount;
//    colliderDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
//    colliderDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//    colliderDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
//    colliderDesc.StructureByteStride = sizeof(FVector4);
//    gEngineLoop.graphicDevice.Device->CreateBuffer(&colliderDesc, nullptr, &m_ColliderBuffer);
//
//    D3D11_SHADER_RESOURCE_VIEW_DESC colliderSRVDesc = {};
//    colliderSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
//    colliderSRVDesc.BufferEx.FirstElement = 0;
//    colliderSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
//    colliderSRVDesc.BufferEx.NumElements = colliderDesc.ByteWidth / colliderDesc.StructureByteStride;
//    gEngineLoop.graphicDevice.Device->CreateShaderResourceView(m_ColliderBuffer, &colliderSRVDesc, &m_ColliderSRV);
//
//    // Culling Result Buffer
//    D3D11_BUFFER_DESC cullingDesc = {};
//    cullingDesc.Usage = D3D11_USAGE_DEFAULT;
//    cullingDesc.ByteWidth = sizeof(float) * renderCount;
//    cullingDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
//    cullingDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
//    cullingDesc.StructureByteStride = sizeof(float);
//    gEngineLoop.graphicDevice.Device->CreateBuffer(&cullingDesc, nullptr, &m_CullingBuffer);
//
//    D3D11_UNORDERED_ACCESS_VIEW_DESC cullingUAVDesc = {};
//    cullingUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
//    cullingUAVDesc.Buffer.FirstElement = 0;
//    cullingUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
//    cullingUAVDesc.Buffer.NumElements = cullingDesc.ByteWidth / cullingDesc.StructureByteStride;
//    gEngineLoop.graphicDevice.Device->CreateUnorderedAccessView(m_CullingBuffer, &cullingUAVDesc, &m_CullingUAV);
//
//    // 결과 복사용 스테이징 버퍼
//    D3D11_BUFFER_DESC resultCopyDesc = {};
//    resultCopyDesc.ByteWidth = renderCount * sizeof(float);
//    resultCopyDesc.Usage = D3D11_USAGE_STAGING;
//    resultCopyDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
//    resultCopyDesc.StructureByteStride = sizeof(float);
//    gEngineLoop.graphicDevice.Device->CreateBuffer(&resultCopyDesc, nullptr, &m_ResultCopyBuffer);
//}
//
//// ComputeShader를 이용한 오클루전 쿼리
//void Culling_Pass::RunOcclusionCullingQuery()
//{
//    UINT renderCount = static_cast<UINT>(CullingRenderMeshList.size());
//    if (renderCount == 0)
//        return;
//
//    // 각 RenderData에 대한 BoundingSphere 정보를 ColliderBuffer에 업데이트
//    // (예: 각 RenderData의 BoundSphere를 Vector4(Center.x, Center.y, Center.z, Radius)로)
//    // -- 업데이트 코드 생략 (맵핑 후 memcpy)
//
//    // ComputeShader 상수 버퍼 업데이트 (카메라, Frustum 등)
//    CB_HizCull cullBuf;
//    CameraData* cam = g_GlobalData->MainCamera_Data;
//    cullBuf.gView = cam->CamView;
//    cullBuf.gProj = cam->CamProj;
//    cullBuf.gViewProj = cam->CamViewProj;
//    cullBuf.gEyePos = Vector3(cam->CamPos.x, cam->CamPos.y, cam->CamPos.z);
//    cullBuf.gViewportSize = Vector2(m_HizViewport.Width, m_HizViewport.Height);
//    m_HizCull_CS->ConstantBufferUpdate(&cullBuf);
//
//    // ComputeShader에 HZB, Collider, Culling 결과 버퍼 바인딩
//    m_HizCull_CS->SetShaderResourceView<gHizMap>(m_HizSRV);
//    m_HizCull_CS->SetShaderResourceView<gColliderBuffer>(m_ColliderSRV);
//    m_HizCull_CS->SetUnorderedAccessView<gCullingBuffer>(m_CullingUAV);
//    m_HizCull_CS->Update();
//
//    // Dispatch ComputeShader (1D 그룹, renderCount 기준)
//    gEngineLoop.graphicDevice.DeviceContext->Dispatch(renderCount, 1, 1);
//}
//
//// GPU 결과를 읽어 오클루전 컬링 결과를 업데이트
//void Culling_Pass::UpdateOcclusionCullingResults()
//{
//    UINT renderCount = static_cast<UINT>(CullingRenderMeshList.size());
//    if (renderCount == 0)
//        return;
//
//    gEngineLoop.graphicDevice.DeviceContext->CopyResource(m_ResultCopyBuffer, m_CullingBuffer);
//    D3D11_MAPPED_SUBRESOURCE mappedResource;
//    gEngineLoop.graphicDevice.DeviceContext->Map(m_ResultCopyBuffer, 0, D3D11_MAP_READ, 0, &mappedResource);
//    memcpy(&m_OcclusionResultList[0], mappedResource.pData, sizeof(float) * renderCount);
//    gEngineLoop.graphicDevice.DeviceContext->Unmap(m_ResultCopyBuffer, 0);
//
//    // 각 RenderData의 m_Draw 플래그 업데이트
//    for (UINT i = 0; i < renderCount; i++)
//    {
//        RenderData* rd = CullingRenderMeshList[i];
//        ObjectData* obj = rd->m_ObjectData;
//        if (obj->IsCull && obj->IsActive)
//        {
//            rd->m_Draw = (bool)m_OcclusionResultList[i];
//        }
//    }
//}
//
//// 메인 렌더 루프 내에서 HZB 오클루전 컬링 패스를 호출
//void Culling_Pass::Render(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
//{
//    // 기존 Render 준비...
//    gEngineLoop.graphicDevice.DeviceContext->RSSetViewports(1, &ActiveViewport->GetD3DViewport());
//    Graphics->ChangeRasterizer(ActiveViewport->GetViewMode());
//    ChangeViewMode(ActiveViewport->GetViewMode());
//    UpdateLightBuffer();
//    UPrimitiveBatch::GetInstance().RenderBatch(ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix());
//
//    // --- HZB Occlusion Culling 패스 시작 ---
//    InitializeOcclusionCulling();         // (최초 한번 또는 리사이즈 시 호출)
//    RenderOccludersHZB();                  // 오클루더 렌더링 및 HZB mip 체인 생성
//    RunOcclusionCullingQuery();            // ComputeShader를 통한 오클루전 쿼리
//    UpdateOcclusionCullingResults();       // GPU 결과를 읽어 Draw 여부 업데이트
//    // --- HZB Occlusion Culling 패스 끝 ---
//
//    // 이제 CullingRenderMeshList에 담긴 객체들 중 m_Draw가 true인 것만 RenderStaticMeshes에서 렌더링하게 함
//    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Primitives))
//        RenderStaticMeshes(World, ActiveViewport);
//    RenderGizmos(World, ActiveViewport);
//    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))
//        RenderBillboards(World, ActiveViewport);
//    RenderLight(World, ActiveViewport);
//
//    ClearRenderArr();
//}
