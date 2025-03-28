﻿#include "WindowsPlatformTime.h"


double FWindowsPlatformTime::GSecondsPerCycle = 0.0;
bool FWindowsPlatformTime::bInitialized = false;

void FWindowsPlatformTime::InitTiming()
{
    if (!bInitialized)
    {
        bInitialized = true;

        double Frequency = static_cast<double>(GetFrequency());
        if (Frequency <= 0.0)
        {
            Frequency = 1.0;
        }

        GSecondsPerCycle = 1.0 / Frequency;
    }
}

float FWindowsPlatformTime::GetSecondsPerCycle()
{
    if (!bInitialized)
    {
        InitTiming();
    }
    return static_cast<float>(GSecondsPerCycle);
}

uint64 FWindowsPlatformTime::GetFrequency()
{
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency);
    return Frequency.QuadPart;
}

double FWindowsPlatformTime::ToMilliseconds(uint64 CycleDiff)
{
    const double Ms = static_cast<double>(CycleDiff)
        * GetSecondsPerCycle()
        * 1000.0;

    return Ms;
}

uint64 FWindowsPlatformTime::Cycles64()
{
    LARGE_INTEGER CycleCount;
    QueryPerformanceCounter(&CycleCount);
    return static_cast<uint64>(CycleCount.QuadPart);
}
