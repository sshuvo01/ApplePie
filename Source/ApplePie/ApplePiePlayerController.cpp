// Fill out your copyright notice in the Description page of Project Settings.


#include "ApplePiePlayerController.h"
#include <Blueprint/UserWidget.h>

AApplePiePlayerController::AApplePiePlayerController()
{

}

void AApplePiePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDOverlayClass)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayClass);
		if (HUDOverlay)
		{
			HUDOverlay->AddToViewport();
		}
	}
}
