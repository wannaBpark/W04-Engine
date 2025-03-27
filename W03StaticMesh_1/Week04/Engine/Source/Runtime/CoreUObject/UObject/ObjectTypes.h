#pragma once


enum OBJECTS
{
	OBJ_SPHERE,
	OBJ_CUBE,
	OBJ_SpotLight,
	OBJ_PARTICLE,
	OBJ_Text,
	OBJ_TRIANGLE,
	OBJ_CAMERA,
	OBJ_PLAYER,
	OBJ_END
};
enum ARROW_DIR
{
	AD_X,
	AD_Y,
	AD_Z,
	AD_END
};
enum ControlMode
{
	CM_TRANSLATION,
	CM_ROTATION,
	CM_SCALE,
	CM_END
};
enum CoordiMode
{
	CDM_WORLD,
	CDM_LOCAL,
	CDM_END
};
enum EPrimitiveColor
{
	RED_X,
	GREEN_Y,
	BLUE_Z,
	NONE,
	RED_X_ROT,
	GREEN_Y_ROT,
	BLUE_Z_ROT
};
