# Week04+ 기술 문서

## Billboard 및 TextRenderer Component 구현

## 개요

`UTextRenderComponent`는 텍스트를 GPU 상에 직접 렌더링하는 **비주얼 컴포넌트**로, 텍스처 아틀라스를 기반으로 문자를 화면에 출력하는 기능을 제공합니다. 본 컴포넌트는 **Billboard 형태로 항상 카메라를 향해 회전**하며, **픽킹(Picking)** 기능과 **UUID 시각화** 등 다양한 에디터 기능을 제공합니다.

---

## 주요 기능

| 기능 | 설명 |
| --- | --- |
| 텍스트 렌더링 | 텍스트 문자열을 텍스처 아틀라스를 기반으로 화면에 출력 |
| 카메라 방향 정렬 | 컴포넌트가 항상 카메라를 향해 회전하도록 렌더링 |
| 버텍스 버퍼 동적 생성 | 문자열 변경 시마다 새로운 버텍스 데이터를 생성 |
| 마우스 픽킹 지원 | 에디터 상에서 마우스로 컴포넌트를 선택 가능 |
| 텍스트 UUID 시각화 | 고유 객체 식별을 위한 색상 코드 렌더링 |

---

## 구성 요소

### 주요 멤버 변수

- `FWString text`: 출력할 텍스트 문자열
- `std::shared_ptr<FTexture> Texture`: 텍스트 렌더링에 사용할 폰트 텍스처
- `TArray<FVertexTexture> vertexTextureArr`: 텍스트를 구성하는 정점 배열
- `ID3D11Buffer* vertexTextBuffer`: GPU에 업로드된 정점 버퍼
- `UINT numTextVertices`: 버텍스 개수
- `int RowCount / ColumnCount`: 텍스처 아틀라스 내 셀 배치 정보

---

## 렌더링 파이프라인

### 1. 텍스트 설정 및 버텍스 생성 (`SetText`)

- 문자열을 순회하면서 각 문자에 대응되는 **UV 좌표**를 계산
- 각 문자를 `quad`로 표현하여 정점들을 구성
- 구성된 정점을 `CreateTextTextureVertexBuffer()`를 통해 GPU에 업로드

### 2. 월드 변환 행렬 생성 (`CreateTextMatrix`)

- 컴포넌트 위치와 스케일 정보를 기반으로 행렬 생성
- 카메라를 향하도록 `ViewMatrix`를 변형하여 billboard 효과 구현

### 3. MVP 렌더링 (`TextMVPRendering`)

- 모델, 뷰, 프로젝션 행렬을 곱한 뒤 상수 버퍼에 업로드
- UUID 색상 인코딩으로 오브젝트 선택 기능과 연결
- 조건부로 텍스트를 `RenderTextPrimitive()`로 출력

---

## 문자별 UV 매핑 방식

텍스트는 아틀라스 텍스처 내에서 셀 단위로 구성됨. 각 문자의 시작 좌표는 `setStartUV()`를 통해 다음과 같이 결정됨:

| 문자 범위 | 시작 좌표 | 위치 계산 방식 |
| --- | --- | --- |
| 대문자 A-Z | (11, 0) | `offset = c - 'A'` |
| 소문자 a-z | (37, 0) | `offset = c - 'a'` |
| 숫자 0-9 | (1, 0) | `offset = c - '0'` |
| 한글 '가'-'힣' | (63, 0) | `offset = c - '가'` |

---

## 마우스 선택 기능 (Picking)

`CheckPickingOnNDC()` 함수는 다음의 단계를 통해 마우스 선택 여부를 판단:

1. 화면 좌표 기준으로 마우스 위치를 -1~1 범위로 정규화
2. 텍스트 버텍스들을 MVP 변환 후 NDC 공간으로 변환
3. 마우스 위치가 사각형 범위 안에 있는지 검사
4. 평균 Z값을 바탕으로 뷰 공간 Z 비교로 픽킹 결정

---

## 사용 예시

```cpp
cpp
복사편집
// 텍스트 설정 및 위치 지정
UTextRenderComponent* uuidComp = new UTextRenderComponent();
uuidComp->SetTexture(L"FontAtlas.png");
uuidComp->SetRowColumnCount(16, 8);

wchar_t buffer[64];
swprintf(buffer, 64, L"UUID: %d", GetOwner()->GetUUID());
uuidComp->SetText(buffer);
uuidComp->SetRelativeLocation(FVector(0, 0, 100));

```

---

## 리소스 해제

소멸자 `~UTextRenderComponent()`에서 `vertexTextBuffer`를 안전하게 해제하여 메모리 누수를 방지합니다.

# 오브젝트 복제

`Duplicate()`함수를 호출하게되면, `Super::Duplicate()`를 계속 타고가서, 최종적으로 UObejct::Duplicate에서 클래스를 복제하게 됩니다.

```cpp
UObject* UObject::Duplicate()
{
    UClass* ClassInfo = GetClass();

    const uint32 Id = UEngineStatics::GenUUID();
    const FString Name = ClassInfo->GetName() + "_" + std::to_string(Id);

    void* RawMemory = FPlatformMemory::Malloc<EAT_Object>(ClassInfo->GetClassSize());
    std::memcpy(RawMemory, this, ClassInfo->GetClassSize());
    std::memset(static_cast<char*>(RawMemory)+8, 0, ClassInfo->GetClassSize()-8); // vtable 제외 나머지 메모리 초기화
    UObject* NewObject = static_cast<UObject*>(RawMemory);
    NewObject->ClassPrivate = ClassInfo;
    NewObject->NamePrivate = Name;
    NewObject->UUID = Id;

    GUObjectArray.AddObject(NewObject);
    return NewObject;
}
```

오브젝트 복제 순서는

1. DECLARE_CLASS로 얻은 클래스의 Size정보를 바탕으로 공간을 먼저 할당합니다.
2. vtable 복사를 위해서 `this`를 `std::memcpy`로 얕은 복사를 수행합니다.
3. vtable을 제외한 모든 공간을 0으로 채웁니다.
4. 나머지 기본정보를 설정하고, 호출스택을 거꾸로 올라가면서 값을 설정합니다.

## 언리얼에서의 UEditorEngine

### UEngine과 UEditorEngine의 관계

- UEngine
    - **1. 게임 루프 관리**
        - `UEngine::Tick()`
            - 게임 실행 시 매 프레임마다 호출되어 게임 로직을 업데이트
                
                ```cpp
                void UEngine::Tick(float DeltaSeconds)
                {
                    // 월드와 각 서브시스템 업데이트
                    for (UWorld* World : Worlds)
                    {
                        World->Tick(LEVELTICK_All, DeltaSeconds);
                    }
                }
                ```
                
    - **2. 렌더링 관리**
        - UGameViewportClient를 통해 게임 화면 렌더링 / 사용자 입력 처리
            - Init() : 게임 초기화
            - StartGameInstance() : 게임 인스턴스 실행
- UEditorEngine
    - `UEditorEngine::Init()`
        - **1. 에디터 구동 시 초기화**
            - 뷰포트 생성, 에디터 명령어 바인딩, 서브시스템 초기화
        - **2. 에디터 명령어 처리**
            - `UEditorEngine::Exec()`
                - 사용자가 입력한 콘솔 명령어를 처리
                
                ```cpp
                bool UEditorEngine::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
                {
                    if (FParse::Command(&Cmd, TEXT("PIE")))
                    {
                        StartPIE();
                        return true;
                    }
                    return false;
                }
                ```
                
        - 3. 플레이 모드 관리
            - PIE, SIE (Play/Simulate In Editor) 모드 지원
                - `StartPIE()` / `EndPIE()`
            
            | 구분 | UEngine | UEditorEngine |
            | --- | --- | --- |
            | 실행 환경 | 게임 실행 시 | 에디터 구동 시 |
            | 주요 모드 | Standalone, Server, Client | 에디터, PIE, SIE |
            | 주로 처리하는 작업 | 게임 루프, 렌더링, 입력 처리 | 에디터 상태 관리, 명령어 처리, 뷰포트 갱신 |
            | 주요 클래스 | UGameEngine, UGameViewportClient | UEditorViewportClient, UEditorSubsystem |
            | 명령어 처리 | 게임 콘솔 명령어 | 에디터 전용 명령어 (`Exec()` 메서드 재정의) |
            | 플레이어 관리 방식 | 게임 모드와 게임 인스턴스를 통해 관리 | PIE 모드에서 별도 관리 |
            
            ### 🌟 **공통점**
            
            | 공통점 | 설명 |
            | --- | --- |
            | 엔진 루프 관리 | `Tick()` 함수로 프레임별 업데이트를 수행 |
            | 월드 관리 | `UWorld` 객체를 사용하여 월드 데이터를 관리 |
            | 기본 엔진 기능 | `UEngine`에서 상속하여 공통 엔진 초기화와 설정 관리 |
            | 서브시스템 관리 | 에디터와 게임 모드에서 각각의 서브시스템을 관리 |
    
    ---
    
    ### Class Default Object란
    
    > - Unreal Engine에서 클래스의 기본 인스턴스
    - UObject기반 클래스를 메모리에 로드할 때 가장 먼저 생성되는 객체
    - 클래스의 기본 속성 값을 저장 역할 = **클래스의 Prototype**
    > 
    
    UObject를 상속한(기반) 클래스가 로드될 때 or 처음 사용될 때 CDO가 생성됨
    
    - StaticClass()를 호출할 때 메모리에 로드됨 (CDO 자체도 UObject)
    
    ```cpp
    UObject* UClass::GetDefaultObject() const
    {
        if (!ClassDefaultObject)
        {
            const_cast<UClass*>(this)->CreateDefaultObject();
        }
        return ClassDefaultObject;
    }
    ----------------------------------- 사용 예시 --------------------------
    UObject* CDO = MyClass->GetDefaultObject();
    AMyCharacter* CDO = AMyCharacter::StaticClass()->GetDefaultObject<AMyCharacter>();
    AMyCharacter::AMyCharacter()
    {
        // CDO를 통해 기본값을 설정 (해당 모든 클래스 인스턴스에 공통으로 적용됨)
        // 단, 인스턴스 화 전에만 값을 수정하는 것이 바람직 / 읽기 전용으로 사용 요망.
        AMyCharacter* CDO = AMyCharacter::StaticClass()->GetDefaultObject<AMyCharacter>();
        CDO->MaxSpeed = 500.f;
    }
    ```
    

### FWorldContext

> Unreal Engine에서 여러 개의 `UWorld` 인스턴스를 관리하기 위해 사용되는 구조체
**특히 에디터 환경에서는.**.
- PIE 인스턴스 관리와 월드 전환 등을 효과적으로 처리함
- UEditorEngine 뿐만 아니라 UGameEngine 등 여러 엔진 클래스에서도 사용됨
> 
- 게임 테크랩을 배운 사람이라면 알아야할 상식 🆕
    - `UEditorEngine` 은 `FWorldContext`를 통해 여러 개의 월드 상태를 관리
        - PIE (Play In Editor)
            - 에디터에서 게임 실행 시, 각 인스턴스는 별도의 FWorldContext로 관리됨
            - 여러 PIE 인스턴스를 통해 멀티플레이어 환경을 테스트 가능
        
        UEditorEngine에서 월드 찾기
        
        ```cpp
        UWorld* UEditorEngine::GetWorldFromPIEInstance(int32 PIEInstanceID) const
        {
            for (const FWorldContext& Context : WorldList)
            {
                if (Context.WorldType == EWorldType::PIE && Context.PIEInstance == PIEInstanceID)
                {
                    return Context.World();
                }
            }
            return nullptr;
        }
        ```
        

### 📑 **[핵심 변수들]**

| 변수명 | 타입 | 설명 |
| --- | --- | --- |
| `WorldType` | `EWorldType::Type` | 월드 타입 (예: 게임, PIE, 에디터 등) |
| `ContextHandle` | `FName` | 월드 컨텍스트를 식별하기 위한 핸들 |
| `LoadedLevelsForPendingMapChange` | `TArray<TObjectPtr<ULevel>>` | 이미 로드된 레벨 목록 |
| `GameViewport` | `UGameViewportClient*` | 현재 사용 중인 게임 뷰포트 |
| `PIEInstance` | `int32` | PIE 인스턴스 ID (-1: 기본) |
| `ThisCurrentWorld` | `TObjectPtr<UWorld>` | 현재 사용 중인 월드 포인터 |