// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ApplePieCharacter.generated.h"

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_9mm UMETA(DisplayName = "9mm"),
	EAT_AR UMETA(Displayname = "AssaultRifle"),

	EAT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class APPLEPIE_API AApplePieCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AApplePieCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	/**
	* Called when the fired button is pressed
	*/
	void FireWeapon();

	/**
	* Called via input to turn at a give rate
	* @param Rate This is a normalized rate
	*/
	void TurnAtRate(float Rate);

	/**
	* Called via input to turn up and down at a given rate
	*/
	void LookUpAtRate(float Rate);


	void Turn(float Value);

	void LookUp(float Value);

	/**
	* Get the 
	*/
	bool GetWeaponFireImpactLocation(const FVector& SocketLocation, FVector& OutImpactLocation) const;

	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation) const;

	void InitializeAmmoMap();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess="true"), Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Camera)
	float BaseLookUpRate;

	/** Turn rate when not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Camera)
	float HipTurnRate;

	/** Turn rate when aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Camera)
	float AimTurnRate;

	/** Look up rate when not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Camera)
	float HipLookUpRate;

	/** Look up rate when aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Camera)
	float AimLookUpRate;

	/** Mouse turn rate while not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ClampMin = 0.f, ClampMax = 1.f), Category = Camera)
	float MouseHipTurnRate;

	/** Mouse turn rate while aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ClampMin = 0.f, ClampMax = 1.f), Category = Camera)
	float MouseAimingTurnRate;

	/** Mouse look up rate while not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ClampMin = 0.f, ClampMax = 1.f), Category = Camera)
	float MouseHipLookUpRate;

	/** Mouse look up rate while aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ClampMin = 0.f, ClampMax = 1.f), Category = Camera)
	float MouseAimingLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	class USoundCue* FireSound;

	/* Flash spawned at weapon barrel */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	class UParticleSystem* MuzzleFlash;

	/** Montage for firing the weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	class UAnimMontage* HipFireMontage;

	/* FX on the bullet impact point */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	class UParticleSystem* ImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	class UParticleSystem* TrailEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Combat)
	bool bAiming;
	
	/** How fast aim zoom is reached */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Combat)
	float AimingSpeed;

	/** Default field of view */
	float DefaultFOV;

	/** Field of view when zoomed in */
	float ZoomedFOV;

	/** Current FOV */
	float CurrentFOV;

	/** Crosshair spread variables */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = Crosshair)
	float CrosshairSpread;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = Crosshair)
	float CrosshairVelocityFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = Crosshair)
	float CrosshairAimFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = Crosshair)
	float CrosshairShootingFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = Crosshair)
	float CrosshairInAirFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = Combat)
	class AApplePieItem* LastTraceHitItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = Combat)
	class AApplePieItem* TraceHitItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = Combat)
	class AApplePieWeapon* EquippedWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = Combat)
	TSubclassOf<AApplePieWeapon> DefaultWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = Combat)
	float CameraInterpDistance;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = Combat)
	float CameraInterpElevation;

	float ShootTimeDuration;
	bool bFiringBullet;
	FTimerHandle CrosshairShootTimer;

	/** True when fire button is held down */
	bool bFireButtonDown;

	/** True when we can fire. False when waiting for the timer. */
	bool bShouldFire;

	/** Rate of automatic gun fire */
	float AutomaticFireRate;

	/** Sets a timer between gunshots */
	FTimerHandle AutoFireTimer;

	void AimingButtonPressed();
	void AimingButtonReleased();
	void CalculateCrosshairSpread(float DeltaTime);

	void FireButtonPressed();
	void FireButtonReleased();

	void StartFireTimer();

	bool bTraceForItems = false;
	int32 OverlappedItemCount = 0;

	UFUNCTION()
	void AutoFireReset();
	
	void StartCrosshairBulletFire();

	UFUNCTION()
	void FinishCrosshairBulletFire();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = Items)
	TMap<EAmmoType, int32> AmmoMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = Items)
	int32 Starting9mmAmmoCount = 85;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = Items)
	int32 StartingARAmmoCount = 120;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool IsAiming() const { return bAiming; }

	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = true), Category = Crosshair)
	float GetCrosshairSpread() const;

	FORCEINLINE int32 GetOverlappedItemCount() const { return OverlappedItemCount; }

	void IncrementOverlappedItemCount(int32 Amount);
	
	/**  */
	class AApplePieWeapon* SpawnDefaultWeapon() const;

	/**  */
	void EquipWeapon(class AApplePieWeapon* Weapon);

	void DropEquippedWeapon();
	void SwapWeapon(AApplePieWeapon* WeaponToSwap);
	void SelectButtonPressed();
	void SelectButtonReleased();
	FVector GetCameraInterpLocation() const;

	void GetPickupItem(AApplePieItem* Item);
};
