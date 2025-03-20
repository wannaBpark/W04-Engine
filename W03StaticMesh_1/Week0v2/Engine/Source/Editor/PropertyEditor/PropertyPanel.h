#pragma once
#include<Windows.h>
#include "Define.h"
class UWorld;

class PropertyPanel
{
private:
	PropertyPanel();

public:
	~PropertyPanel();
	static PropertyPanel& GetInstance()
	{
		static PropertyPanel Inst;
		return Inst;
	}
	void OnResize(HWND hWnd);
	void Draw(UWorld* world);
   void RGBToHSV(float r, float g, float b, float& h, float& s, float& v); 
   inline void HSVToRGB(float h, float s, float v, float& r, float& g, float& b)
   {
	   // h: 0~360, s:0~1, v:0~1
	   float c = v * s;
	   float hp = h / 60.0f;             // 0~6 ±¸°£
	   float x = c * (1.0f - fabsf(fmodf(hp, 2.0f) - 1.0f));
	   float m = v - c;

	   if (hp < 1.0f) { r = c;  g = x;  b = 0.0f; }
	   else if (hp < 2.0f) { r = x;  g = c;  b = 0.0f; }
	   else if (hp < 3.0f) { r = 0.0f; g = c;  b = x; }
	   else if (hp < 4.0f) { r = 0.0f; g = x;  b = c; }
	   else if (hp < 5.0f) { r = x;  g = 0.0f; b = c; }
	   else { r = c;  g = 0.0f; b = x; }

	   r += m;  g += m;  b += m;
   }


private:
	bool bWasOpen = true;
	UINT width;
	UINT height;
};