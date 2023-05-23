// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ApplePieItem.h"
#include "ApplePieWeapon.generated.h"

/**
 * 
 */
UCLASS()
class APPLEPIE_API AApplePieWeapon : public AApplePieItem
{
	GENERATED_BODY()
public:
	AApplePieWeapon();
	void Throw();

protected:
	void StopFalling();

	virtual void Tick(float DeltaTime) override;
private:
	FTimerHandle ThrowTimerHandle;
	float ThrowTime;
	bool bFalling;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//int32 AmmoCount;
};
