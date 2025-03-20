#pragma once
#include "UText.h"
class UTextUUID :
    public UText
{
public:
	UTextUUID();
	~UTextUUID();

	virtual void		Render()					override;
	virtual int			CheckRayIntersection(FVector& rayOrigin,
		FVector& rayDirection, float& pfNearHitDistance) override;
	void SetUUID(uint32 UUID);
private:
};

