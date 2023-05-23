// Fill out your copyright notice in the Description page of Project Settings.


#include "ApplePieCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "ApplePieItem.h"
#include "ApplePieWeapon.h"

#include <Components/SphereComponent.h>
#include <Components/BoxComponent.h>

// Sets default values
AApplePieCharacter::AApplePieCharacter()
	: BaseTurnRate{ 45.f },
	BaseLookUpRate{ 45.f },
	bAiming{ false },
	DefaultFOV{ 0.f },
	ZoomedFOV{ 60.f },
	CurrentFOV{ 0.f },
	AimingSpeed{ 25.f },
	// Crosshair spread factors
	CrosshairSpread{ 0.f },
	CrosshairVelocityFactor{ 0.f },
	CrosshairInAirFactor{ 0.f },
	CrosshairAimFactor{ 0.f },
	CrosshairShootingFactor{ 0.f },
	// Bullet fire timer variables
	ShootTimeDuration{ 0.05f },
	bFiringBullet{ false },
	AutomaticFireRate{ 0.1f },
	bShouldFire{ true },
	bFireButtonDown{ false },
	bTraceForItems{ false },
	// Camera interp location variables
	CameraInterpDistance{ 250.f },
	CameraInterpElevation{ 65.f }
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector{ 0.f, 50.f, 50.f };

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; 

	// Don't rotate when the controller rotates
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	check(MovementComponent);
	MovementComponent->bOrientRotationToMovement = false;
	MovementComponent->RotationRate = FRotator{ 0.f, 540.f, 0.f };
	MovementComponent->JumpZVelocity = 600.f;
	MovementComponent->AirControl = 0.2f;
}

// Called when the game starts or when spawned
void AApplePieCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (FollowCamera)
	{
		DefaultFOV = GetFollowCamera()->FieldOfView;
	}

	EquipWeapon(SpawnDefaultWeapon());
	InitializeAmmoMap();
}

// Called every frame
void AApplePieCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, ZoomedFOV, DeltaTime, AimingSpeed);
		BaseTurnRate = AimTurnRate;
		BaseLookUpRate = AimLookUpRate;
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, AimingSpeed);
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}

	GetFollowCamera()->SetFieldOfView(CurrentFOV);
	CalculateCrosshairSpread(DeltaTime);

	if (bTraceForItems)
	{
		FHitResult ItemHitResult;
		FVector ItemHitLocation;
		TraceUnderCrosshairs(ItemHitResult, ItemHitLocation);
		if (ItemHitResult.bBlockingHit)
		{
			TraceHitItem = Cast<AApplePieItem>(ItemHitResult.Actor);
			if (TraceHitItem)
			{
				TraceHitItem->ShowPickupWidget();
			}

			if (LastTraceHitItem && LastTraceHitItem != TraceHitItem)
			{
				LastTraceHitItem->HidePickupWidget();
			}

			LastTraceHitItem = TraceHitItem;
		}
		else
		{
			TraceHitItem = nullptr;
		}
	}
	else if (LastTraceHitItem)
	{
		LastTraceHitItem->HidePickupWidget();
	}
}

// Called to bind functionality to input
void AApplePieCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ThisClass::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ThisClass::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("TurnRate"), this, &ThisClass::TurnAtRate);
	PlayerInputComponent->BindAxis(TEXT("LookUpRate"), this, &ThisClass::LookUpAtRate);
	//PlayerInputComponent->BindAxis(TEXT("Turn"), this, &APawn::AddControllerYawInput);
	//PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ThisClass::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ThisClass::LookUp);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction(TEXT("FireButton"), IE_Pressed, this, &ThisClass::FireButtonPressed);
	PlayerInputComponent->BindAction(TEXT("FireButton"), IE_Released, this, &ThisClass::FireButtonReleased);

	PlayerInputComponent->BindAction(TEXT("AimingButton"), IE_Pressed, this, &ThisClass::AimingButtonPressed);
	PlayerInputComponent->BindAction(TEXT("AimingButton"), IE_Released, this, &ThisClass::AimingButtonReleased);

	PlayerInputComponent->BindAction(TEXT("Select"), IE_Pressed, this, &ThisClass::SelectButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Select"), IE_Released, this, &ThisClass::SelectButtonReleased);
}


void AApplePieCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.f)
	{
		const FRotator ControlRotation{ Controller->GetControlRotation() };
		const FRotator ControlYawRotation{ 0.f, ControlRotation.Yaw, 0.f };

		const FVector ForwardDirection{ FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::X) };
		AddMovementInput(ForwardDirection, Value);
	}
}

void AApplePieCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0.f)
	{
		const FRotator ControlRotation{ Controller->GetControlRotation() };
		const FRotator ControlYawRotation{ 0.f, ControlRotation.Yaw, 0.f };

		const FVector RightDirection{ FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::Y) };
		AddMovementInput(RightDirection, Value);
	}
}

void AApplePieCharacter::FireWeapon()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	if (const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName(TEXT("BarrelSocket")))
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector HitLocation;
		if (GetWeaponFireImpactLocation(SocketTransform.GetLocation(), HitLocation))
		{
			if (ImpactEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, HitLocation);
			}

			if (TrailEffect)
			{
				UParticleSystemComponent* ParticleComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TrailEffect, SocketTransform);
				check(ParticleComponent);
				ParticleComponent->SetVectorParameter(FName{ TEXT("Target") }, HitLocation);
			}
		}
		/*
		FVector2D ViewportSize;
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(ViewportSize);
		}

		// TODO: Find a way to not hardcode this
		const FVector2D CrossHairScreenLocation{ ViewportSize.X / 2.f + 50.f, ViewportSize.Y / 2.f - 50.f };
		FVector CrosshairWorldLocation, CrosshairWorldDirection;
		if (UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), 
			CrossHairScreenLocation, CrosshairWorldLocation, CrosshairWorldDirection))
		{
			FHitResult CrosshairTraceHitResult;
			//const FVector StartLocation{ SocketTransform.GetLocation() };
			const FVector StartLocation{ CrosshairWorldLocation };
			//const FQuat StartRotation{ SocketTransform.GetRotation() };
			//const FVector Direction{ StartRotation.GetAxisX() };
			const FVector Direction{ CrosshairWorldDirection };
			const FVector EndLocation{ StartLocation + Direction * 50'000.f };
			FVector TrailEffectTarget{ EndLocation };
			GetWorld()->LineTraceSingleByChannel(CrosshairTraceHitResult, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility);
			
			if (CrosshairTraceHitResult.bBlockingHit)
			{
				//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.f);
				//DrawDebugPoint(GetWorld(), HitResult.Location, 10.f, FColor::Red, false, 2.f);
				TrailEffectTarget = CrosshairTraceHitResult.ImpactPoint;
			}

			// Perform a second trace from the gun barrel
			FHitResult WeaponTraceHit;
			const FVector WeaponTraceStart{ SocketTransform.GetLocation() };
			const FVector WeaponTraceEnd{ TrailEffectTarget };
			GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

			if (WeaponTraceHit.bBlockingHit)
			{
				TrailEffectTarget = WeaponTraceHit.ImpactPoint;
			}

			if (ImpactEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, TrailEffectTarget);
			}

			if (TrailEffect)
			{
				UParticleSystemComponent* ParticleComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TrailEffect, SocketTransform);
				check(ParticleComponent);
				ParticleComponent->SetVectorParameter(FName{ TEXT("Target") }, TrailEffectTarget);
			}
		}
		*/
	}
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(TEXT("StartFire"));
	}

	// Start bullet fire time for crosshair
	StartCrosshairBulletFire();
}

void AApplePieCharacter::AimingButtonPressed()
{
	bAiming = true;
}
void AApplePieCharacter::AimingButtonReleased()
{
	bAiming = false;
}

bool AApplePieCharacter::GetWeaponFireImpactLocation(const FVector& SocketLocation, FVector& OutImpactLocation) const
{
	/*
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// TODO: Find a way to not hardcode this
	const FVector2D CrossHairScreenLocation{ ViewportSize.X / 2.f + 50.f, ViewportSize.Y / 2.f - 50.f };
	FVector CrosshairWorldLocation, CrosshairWorldDirection;

	if (UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrossHairScreenLocation, CrosshairWorldLocation, CrosshairWorldDirection))
	{
		FHitResult ScreenTraceHitResult;
		const FVector ScreenTraceStartLocation{ CrosshairWorldLocation };
		const FVector ScreenTraceEndLocation{ ScreenTraceStartLocation + CrosshairWorldDirection * 50'000.f };
		OutImpactLocation = ScreenTraceEndLocation;
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHitResult, ScreenTraceStartLocation, ScreenTraceEndLocation, ECollisionChannel::ECC_Visibility);

		if (ScreenTraceHitResult.bBlockingHit)
		{
			OutImpactLocation = ScreenTraceHitResult.ImpactPoint;
		}

		// Perform a second trace from the gun barrel
		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart{ SocketLocation };
		const FVector WeaponTraceEnd{ OutImpactLocation };
		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

		if (WeaponTraceHit.bBlockingHit)
		{
			OutImpactLocation = WeaponTraceHit.ImpactPoint;
		}

		return true;
	}
	*/
	FHitResult CrosshairsHitResult;
	if (TraceUnderCrosshairs(CrosshairsHitResult, OutImpactLocation))
	{
		// Perform a second trace from the gun barrel
		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart{ SocketLocation };
		const FVector WeaponTraceEnd{ OutImpactLocation };
		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

		if (WeaponTraceHit.bBlockingHit)
		{
			OutImpactLocation = WeaponTraceHit.ImpactPoint;
		}

		return true;
	}

	return false;
}

bool AApplePieCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation) const
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	const FVector2D CrossHairScreenLocation{ ViewportSize.X / 2.f + 50.f, ViewportSize.Y / 2.f - 50.f };
	FVector CrosshairWorldLocation, CrosshairWorldDirection;

	if (UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrossHairScreenLocation, CrosshairWorldLocation, CrosshairWorldDirection))
	{
		const FVector ScreenTraceStartLocation{ CrosshairWorldLocation };
		const FVector ScreenTraceEndLocation{ ScreenTraceStartLocation + CrosshairWorldDirection * 50'000.f };
		OutHitLocation = ScreenTraceEndLocation;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, ScreenTraceStartLocation, ScreenTraceEndLocation, ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.ImpactPoint;
			return true;
		}
	}

	return false;
}

void AApplePieCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmoCount);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmoCount);
}

void AApplePieCharacter::TurnAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); // deg/second * second/frame
}

void AApplePieCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds()); // deg/second * second/frame
}

void AApplePieCharacter::Turn(float Value)
{
	const float MouseTurnScaleFactor = bAiming ? MouseAimingTurnRate : MouseHipTurnRate;
	AddControllerYawInput(Value * MouseTurnScaleFactor);
}

void AApplePieCharacter::LookUp(float Value)
{
	const float MouseLookUpScaleFactor = bAiming ? MouseAimingLookUpRate : MouseHipLookUpRate;
	AddControllerPitchInput(Value * MouseLookUpScaleFactor);
}

float AApplePieCharacter::GetCrosshairSpread() const
{
	return CrosshairSpread;
}

void AApplePieCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	const FVector2D WalkSpeedRange{ 0.f, 600.f };
	const FVector2D VelocityRange{ 0.f, 1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.f;

	if (GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}

	if (bAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.45f, DeltaTime, 30.f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}

	if (bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);
	}

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityRange, Velocity.Size());
	CrosshairSpread = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairShootingFactor - CrosshairAimFactor;
}

void AApplePieCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &ThisClass::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AApplePieCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AApplePieCharacter::FireButtonPressed()
{
	bFireButtonDown = true;
	StartFireTimer();
}

void AApplePieCharacter::FireButtonReleased()
{
	bFireButtonDown = false;
}

void AApplePieCharacter::StartFireTimer()
{
	if (bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AApplePieCharacter::AutoFireReset, AutomaticFireRate);
	}
}

void AApplePieCharacter::AutoFireReset()
{
	bShouldFire = true;
	if (bFireButtonDown)
	{
		StartFireTimer();
	}
}

void AApplePieCharacter::IncrementOverlappedItemCount(int32 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bTraceForItems = true;
	}
}

AApplePieWeapon* AApplePieCharacter::SpawnDefaultWeapon() const
{
	// RightHandSocket
	if (DefaultWeaponClass)
	{
		return GetWorld()->SpawnActor<AApplePieWeapon>(DefaultWeaponClass);
	}

	return nullptr;
}

void AApplePieCharacter::EquipWeapon(AApplePieWeapon* Weapon)
{
	if (Weapon)
	{
		/*
		if (UBoxComponent* Collider = Weapon->GetCollider())
		{
			Collider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		
		if (USphereComponent* PickupRadius = Weapon->GetPickupRadiusSphere())
		{
			PickupRadius->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		*/

		if (const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(TEXT("RightHandSocket")))
		{
			HandSocket->AttachActor(Weapon, GetMesh());
			EquippedWeapon = Weapon;
			EquippedWeapon->SetState(EItemState::EIS_Equipped);
		}
	}
}

void AApplePieCharacter::DropEquippedWeapon()
{
	if (EquippedWeapon)
	{
		if (USkeletalMeshComponent* WeaponMesh = EquippedWeapon->GetMesh())
		{
			const FDetachmentTransformRules DetachmentRules{ EDetachmentRule::KeepWorld, true };// ();
			WeaponMesh->DetachFromComponent(DetachmentRules);
			EquippedWeapon->SetState(EItemState::EIS_Falling);
			EquippedWeapon->Throw();
		}
	}
}

void AApplePieCharacter::SwapWeapon(AApplePieWeapon* WeaponToSwap)
{
	DropEquippedWeapon();
	EquipWeapon(WeaponToSwap);
}

void AApplePieCharacter::SelectButtonPressed()
{
	if (AApplePieWeapon* Weapon = Cast<AApplePieWeapon>(TraceHitItem))
	{
		//SwapWeapon(Cast<AApplePieWeapon>(TraceHitItem));
		Weapon->StartInterping(this);
		TraceHitItem = LastTraceHitItem = nullptr;
	}
}

void AApplePieCharacter::SelectButtonReleased()
{

}

FVector AApplePieCharacter::GetCameraInterpLocation() const
{
	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
	const FVector CameraForwardVector{ FollowCamera->GetForwardVector() };

	return CameraWorldLocation + CameraForwardVector * CameraInterpDistance
		+ FVector::UpVector * CameraInterpElevation;
}

void AApplePieCharacter::GetPickupItem(AApplePieItem* Item)
{
	AApplePieWeapon* Weapon = Cast<AApplePieWeapon>(Item);
	if (Weapon)
	{
		SwapWeapon(Weapon);
	}
}