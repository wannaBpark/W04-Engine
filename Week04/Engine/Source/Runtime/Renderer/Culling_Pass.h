//// HZB Occlusion Culling Implementation
//
//#include <iostream>
//#pragma comment(lib, "user32")
//#pragma comment(lib, "d3d11")
//#pragma comment(lib, "d3dcompiler")
//
//#define _TCHAR_DEFINED
//#include <d3d11.h>
//#include "EngineBaseTypes.h"
//#include "Define.h"
//#include "Container/Set.h"
//#include "Define.h"
//
//// HZB 관련 상수 (예: 512x256 해상도, LOOSE_FACTOR는 HZB와 직접적 관련은 없으므로 HZB용은 고정)
//inline static constexpr float HZB_WIDTH = 512.0f;
//inline static constexpr float HZB_HEIGHT = 256.0f;
//
//class Culling_Pass
//{
//public:
//    ID3D11Texture2D* m_HizBuffer = nullptr;
//    ID3D11ShaderResourceView* m_HizSRV = nullptr;
//    std::vector<ID3D11RenderTargetView*> m_HizRTVs;
//    std::vector<ID3D11ShaderResourceView*> m_HizSRVs;
//    UINT m_MaxMipLevel = 0;
//    ID3D11CommandList* m_MipCommandList = nullptr;
//    ID3D11DepthStencilView* m_HizDSV = nullptr;
//    D3D11_VIEWPORT m_HizViewport = {};
//
//    // Occlusion Culling용 버퍼들
//    ID3D11Buffer* m_ColliderBuffer = nullptr;
//    ID3D11ShaderResourceView* m_ColliderSRV = nullptr;
//    ID3D11Buffer* m_CullingBuffer = nullptr;
//    ID3D11UnorderedAccessView* m_CullingUAV = nullptr;
//    ID3D11Buffer* m_ResultCopyBuffer = nullptr;
//
//    // CPU측 임시 결과 저장
//    std::vector<float> m_OcclusionResultList;
//
//    // 대상 렌더 데이터 (컬링 대상)
//    std::vector<FVertexSimple*> CullingRenderMeshList;
//
//    // HZB Occlusion Compute Shader
//    ID3D11ComputeShader* m_HizCull_CS = nullptr;
//
//    // ---------- HZB Occlusion Culling 관련 메서드 ----------
//    void InitializeOcclusionCulling(); // HZB 자원 생성 및 뷰포트 설정
//    void RenderOccludersHZB();         // Occluder 메시들을 HZB에 렌더링
//    void GenerateHZBMipChain();        // MipChain 생성 (CommandList 실행)
//    void CreateOcclusionBuffers();     // Collider, Culling, Result 버퍼 생성
//    void RunOcclusionCullingQuery();   // ComputeShader Dispatch
//    void UpdateOcclusionCullingResults(); // GPU 결과 읽어 Draw 여부 결정
//};