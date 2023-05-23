// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ApplePiePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class APPLEPIE_API AApplePiePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AApplePiePlayerController();
protected:
	void BeginPlay() override;

private:
	/** Reference to the overall hud overlay blueprint class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = true))
	TSubclassOf<class UUserWidget> HUDOverlayClass;

	/** Variable to hold the hud overlay widget afater creating it */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = true))
	UUserWidget* HUDOverlay;
};
