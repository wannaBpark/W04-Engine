#pragma once

#define DECLARE_CLASS(ClassName, ParentClassName) \
public: \
    static UClass* StaticClass() { \
        static UClass ClassInfo(#ClassName, ParentClassName::StaticClass()); \
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