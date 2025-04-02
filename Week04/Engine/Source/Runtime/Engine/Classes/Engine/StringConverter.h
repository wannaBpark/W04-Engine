#pragma once
#include <windows.h>
#include <iostream>

class StringConverter {
    // 함수 선언 및 정의

public:
    static std::string ConvertWCharToChar(const wchar_t* wstr)
    {
        if (wstr == nullptr)
            return "";

        // 필요한 버퍼 크기 계산
        int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
        if (bufferSize == 0)
            return "";

        std::string result(bufferSize, 0);
        // 변환 수행
        WideCharToMultiByte(CP_UTF8, 0, wstr, -1, result.data(), bufferSize, nullptr, nullptr);

        return result;
    }
};