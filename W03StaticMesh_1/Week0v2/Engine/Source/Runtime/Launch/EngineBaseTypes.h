#pragma once

enum EViewModeIndex
{
    VMI_Lit,
    VMI_Unlit,
    VMI_Wireframe,
};


enum ELevelViewportType
{
    /** Top */
    LVT_OrthoXY = 0,
    /** Front */
    LVT_OrthoXZ = 1,
    /** Left */
    LVT_OrthoYZ = 2,
    LVT_Perspective = 3,
    //LVT_OrthoFreelook = 4,
    /** Bottom */
    LVT_OrthoNegativeXY,
    /** Back */
    LVT_OrthoNegativeXZ,
    /** Right */
    LVT_OrthoNegativeYZ,
    LVT_MAX,

    LVT_None = 255,
};
