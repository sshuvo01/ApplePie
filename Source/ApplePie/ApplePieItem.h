// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ApplePieItem.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged = 0 UMETA(DisplayName = "Damaged"),
	EIR_Common = 1 UMETA(DisplayName = "Common"),
	EIR_Uncommon = 2 UMETA(DisplayName = "Uncommon"),
	EIR_Rare = 3 UMETA(DisplayName = "Rare"),
	EIR_Legendary = 4 UMETA(DisplayName = "Legendary"),

	EIR_MAX = 5 UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_Pickup UMETA(DisplayName = "Pickup"),
	EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),
	EIS_PickedUp UMETA(DisplayName = "PickedUp"),
	EIS_Falling UMETA(DisplayName = "Falling"),

	EIS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class APPLEPIE_API AApplePieItem : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AApplePieItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = true))
	class USkeletalMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = true))
	class UBoxComponent* Collider;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = true))
	class USphereComponent* PickupRadiusSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = true))
	class UWidgetComponent* PickupWidget;

	/** Displayable name for this widget */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = true))
	FString Name;

	/** Count for this item. For example, ammo count. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = true))
	int32 Count;

	/** Rarity of this item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = true))
	EItemRarity Rarity;

	/** State of this item. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = true))
	EItemState State;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = true))
	TArray<bool> RarityStars;

	/** The curve asset to use for the item's Z location when interping. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = true))
	class UCurveFloat* InterpZCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = true))
	class UCurveFloat* InterpScaleCurve;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = true))
	FVector InterpStartLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = true))
	FVector InterpTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = true))
	bool bInterping;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = true))
	class AApplePieCharacter* Character;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = true))
	class USoundCue* PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = true))
	class USoundCue* EquipSound;

	FTimerHandle InterpTimerHandle;

	float InterpZTime;

	float InterpYawOffset;

	void SetRarityStars();
	void SetItemProperties();
	
	void Interpolate(float DeltaTime);
public:
	void ShowPickupWidget();
	void HidePickupWidget();

	UFUNCTION(BlueprintCallable, Category = Properties)
	FString GetName() const { return Name; }

	UFUNCTION(BlueprintCallable, Category = Properties)
	int32 GetCount() const { return Count; }

	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// UPrimitiveComponent*, OverlappedComponent, AActor*, OtherActor, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	TArray<bool> GetRarityStars() const { return RarityStars; }

	void FinishInterping();

	FORCEINLINE UBoxComponent* GetCollider() const { return Collider; }
	FORCEINLINE USphereComponent* GetPickupRadiusSphere() const { return PickupRadiusSphere; }
	void SetState(EItemState InState);
	FORCEINLINE  USkeletalMeshComponent* GetMesh() const { return Mesh; }

	FORCEINLINE EItemState GetState() const { return State; }

	FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound; }
	FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound; }

	void StartInterping(AApplePieCharacter* InCharacter);
};