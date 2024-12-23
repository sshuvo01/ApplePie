// Fill out your copyright notice in the Description page of Project Settings.


#include "ApplePieWeapon.h"

AApplePieWeapon::AApplePieWeapon()
	: ThrowTime{ 0.8f },
	bFalling{ false },
	AmmoType{ EAmmoType::EAT_9mm },
	ReloadMontageSection{ FName{TEXT("Reload SMG")} },
	AmmoCount{ 30 },
	MagazineCapacity{ 30 },
	ClipBoneName{ FName{ TEXT("smg_clip")}}
{
	PrimaryActorTick.bCanEverTick = true;
}

void AApplePieWeapon::ReloadAmmo(int32 InAmmoCount)
{
	checkf(AmmoCount + InAmmoCount <= MagazineCapacity, TEXT("Oh no more than this magazine can carry"));
	AmmoCount += InAmmoCount;
}

void AApplePieWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetState() == EItemState::EIS_Falling && bFalling)
	{
		const FRotator MeshRotation{ 0.f, GetMesh()->GetComponentRotation().Yaw, 0.f };
		GetMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}

	AApplePieWeapon::__PPO__AmmoCount();
}

void AApplePieWeapon::Throw()
{
	USkeletalMeshComponent* WeaponMesh = GetMesh();
	check(WeaponMesh);
	const FRotator MeshRotation{ 0.f, WeaponMesh->GetComponentRotation().Yaw, 0.f };
	WeaponMesh->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector MeshForward{ WeaponMesh->GetForwardVector() };
	const FVector MeshRight{ WeaponMesh->GetRightVector() };
	// Direction in which we throw the weapon
	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);

	const float RandomRotation = FMath::FRandRange(20.f, 50.f);
	ImpulseDirection = MeshRight.RotateAngleAxis(RandomRotation, FVector::UpVector);
	ImpulseDirection *= 2'000.f;
	WeaponMesh->AddImpulse(ImpulseDirection);

	bFalling = true;
	GetWorldTimerManager().SetTimer(ThrowTimerHandle, this, &AApplePieWeapon::StopFalling, ThrowTime);
}

void AApplePieWeapon::StopFalling()
{
	bFalling = false;
	SetState(EItemState::EIS_Pickup);
}

void AApplePieWeapon::DecrementAmmoCount()
{
	if (AmmoCount - 1 < 0)
	{
		AmmoCount = 0;
	}
	else
	{
		AmmoCount--;
	}
}