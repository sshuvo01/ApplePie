// Fill out your copyright notice in the Description page of Project Settings.


#include "ApplePieWeapon.h"

AApplePieWeapon::AApplePieWeapon()
	: ThrowTime{ 0.8f },
	bFalling{ false }
{
	PrimaryActorTick.bCanEverTick = true;
}

void AApplePieWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetState() == EItemState::EIS_Falling && bFalling)
	{
		const FRotator MeshRotation{ 0.f, GetMesh()->GetComponentRotation().Yaw, 0.f };
		GetMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}
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