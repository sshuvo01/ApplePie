// Fill out your copyright notice in the Description page of Project Settings.


#include "ApplePieAnimInstance.h"
#include "ApplePieCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

void UApplePieAnimInstance::UpdateAnimationProperties(float DelatTime)
{
	if (!ApplePieCharacter)
	{
		ApplePieCharacter = Cast<AApplePieCharacter>(TryGetPawnOwner());
	}
	if (ApplePieCharacter)
	{
		FVector Velocity{ ApplePieCharacter->GetVelocity() };
		Velocity.Z = 0.f;
		Speed = Velocity.Size();

		bIsInAir = ApplePieCharacter->GetCharacterMovement()->IsFalling();

		const float AccelerationSize = ApplePieCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size();
		bIsAccelerating = AccelerationSize > 0.f ? true : false;

		FRotator AimRotation = ApplePieCharacter->GetBaseAimRotation();
		FString AimMessage = FString::Printf(TEXT("Base aim yaw: %f"), AimRotation.Yaw);

		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ApplePieCharacter->GetVelocity());
		FString MovementMessage = FString::Printf(TEXT("Movement yaw: %f"), MovementRotation.Yaw);
		
		if (ApplePieCharacter->GetVelocity().Size() > 0.f)
		{
			LastMovementOffsetYaw = MovementOffsetYaw;
		}

		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		
		if (GEngine)
		{
			//GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Blue, AimMessage);
			//GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Red, MovementMessage);
			GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Blue, 
				FString::Printf(TEXT("movement offset yaw: %f"), MovementOffsetYaw ) );
		}

		bAiming = ApplePieCharacter->IsAiming();
	}
}

void UApplePieAnimInstance::NativeInitializeAnimation()
{
	//Super::NativeInitializeAnimation();
	ApplePieCharacter = Cast<AApplePieCharacter>(TryGetPawnOwner());
}