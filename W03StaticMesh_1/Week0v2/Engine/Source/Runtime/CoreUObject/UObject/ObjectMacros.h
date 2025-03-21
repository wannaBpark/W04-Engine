#pragma once

// ��ü Ŭ���� ��ũ��
#define DECLARE_CLASS(TClass, TSuperClass) \
public: \
    using Super = TSuperClass; \
    using ThisClass = TClass; \
    static UClass* StaticClass() { \
        static UClass ClassInfo(#TClass, TSuperClass::StaticClass()); \
        return &ClassInfo; \
    } \
    virtual UClass* GetClass() const override { return StaticClass(); }

//
//#define PROPERTY(type, name) \
//private: \
//    type name; \
//public: \
//    void Set##name(const type& value) { name = value; } \
//    type Get##name() const { return name; }

#define PROPERTY(type, name) \
    void Set##name(const type& value) { name = value; } \
    type Get##name() const { return name; }