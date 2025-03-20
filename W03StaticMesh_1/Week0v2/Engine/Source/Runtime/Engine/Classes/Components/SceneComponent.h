#pragma once
#include "UObject/Object.h"

class USceneComponent : public UObject
{
	DECLARE_CLASS(USceneComponent, UObject)
public:
	USceneComponent();
	~USceneComponent();

	virtual void		Initialize()	override;
	virtual void		Update(double deltaTime)	override;
	virtual void		Release()					override;
	virtual void		Render()					override;
	virtual FVector				GetForwardVector();
	virtual FVector				GetRightVector();
	virtual FVector				GetUpVector();
	void	 			AddLocation(FVector _added);
	void				AddRotation(FVector _added);
	void				AddScale(FVector _added);

	void				AddChild(USceneComponent* _newChild);

protected:
	FVector				RelativeLocation;
	FVector				RelativeRotation;
	FQuat				QuatRotation;
	FVector				RelativeScale3D;
	USceneComponent*	AttachParent = nullptr;
	TArray<USceneComponent*> AttachChildren;

public:
	virtual FVector						GetWorldRotation();
	FVector						GetWorldScale();
	FVector						GetWorldLocation();
	FVector						GetLocalRotation();
	FQuat						GetQuat() {		return QuatRotation;	}

	inline FVector				GetLocalScale() { return RelativeScale3D; }
	inline FVector				GetLocalLocation() { return RelativeLocation; }

	inline void					SetLocation(FVector _newLoc) { RelativeLocation = _newLoc; }
	virtual void				SetRotation(FVector _newRot);
	inline void					SetRotation(FQuat _newRot) { QuatRotation = _newRot; }
	inline void					SetScale(FVector _newScale) { RelativeScale3D = _newScale; }
	inline void					SetParent(USceneComponent* _parent) { AttachParent = _parent; }

private:
	class UTextUUID*				uuidText = nullptr;
public:
	virtual void				RenderUUID() override;
};

