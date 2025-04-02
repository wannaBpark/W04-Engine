#include "UObject/Object.h"
#include "CoreUObject/UObject/UObjectArray.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

extern UEditorEngine* GEditor;

class UEditorEngine : public UEngine
{
    DECLARE_CLASS(UEditorEngine, UEngine)
public:
    UEditorEngine() = default;
    void Initialize();
    virtual void Tick(float DeltaSeconds);
    void Release();
    void StartPIE();
    void EndPIE();

    FWorldContext& GetEditorWorldContext() const;
};
