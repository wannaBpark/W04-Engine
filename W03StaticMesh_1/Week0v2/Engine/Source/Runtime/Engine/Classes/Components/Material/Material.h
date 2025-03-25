#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UMaterial : public UObject {
    DECLARE_CLASS(UMaterial, UObject)

public:
    UMaterial() {}
    ~UMaterial() {}
    PROPERTY(FObjMaterialInfo, materialInfo);
private:
    FObjMaterialInfo materialInfo;
};