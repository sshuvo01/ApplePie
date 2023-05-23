// Fill out your copyright notice in the Description page of Project Settings.


#include "ApplePieItem.h"
#include "ApplePieCharacter.h"
#include <Components/BoxComponent.h>
#include <Components/WidgetComponent.h>
#include <Components/SphereComponent.h>
#include <Camera/CameraComponent.h>

// Sets default values
AApplePieItem::AApplePieItem()
	: Name{ TEXT("Default") },
	Count{ 0 },
	State{ EItemState::EIS_Pickup },
	//
	bInterping{ false },
	InterpZTime(0.7f),
	InterpYawOffset{0.f}
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider"));
	Collider->SetupAttachment(Mesh);
	Collider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Collider->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Pickup Widget"));
	PickupWidget->SetupAttachment(GetRootComponent());
	
	PickupRadiusSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Pickup radius sphere"));
	PickupRadiusSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AApplePieItem::BeginPlay()
{
	Super::BeginPlay();
	
	HidePickupWidget();

	PickupRadiusSphere->OnComponentBeginOverlap.AddDynamic(this, &AApplePieItem::OnSphereBeginOverlap);
	PickupRadiusSphere->OnComponentEndOverlap.AddDynamic(this, &AApplePieItem::OnSphereEndOverlap);

	// Sets the array based on the rarity set
	SetRarityStars();

	// Set item properties
	SetItemProperties();
}

void AApplePieItem::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AApplePieCharacter* OverlappedCharacter = Cast<AApplePieCharacter>(OtherActor))
	{
		OverlappedCharacter->IncrementOverlappedItemCount(1);
	}
}

void AApplePieItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (AApplePieCharacter* OverlappedCharacter = Cast<AApplePieCharacter>(OtherActor))
	{
		OverlappedCharacter->IncrementOverlappedItemCount(-1);
	}
}

// Called every frame
void AApplePieItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	Interpolate(DeltaTime);
}

void AApplePieItem::ShowPickupWidget()
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(true);
	}
}

void AApplePieItem::HidePickupWidget()
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void AApplePieItem::SetRarityStars()
{
	RarityStars.Empty();

	for (int32 i = 0; i < static_cast<int32>(EItemRarity::EIR_MAX) + 1; i++)
	{
		RarityStars.Add(false);
	}

	if (Rarity != EItemRarity::EIR_MAX)
	{
		for (int32 i = 1; i <= static_cast<int32>(Rarity) + 1; i++)
		{
			RarityStars[i] = true;
		}
	}
}

void AApplePieItem::SetItemProperties()
{
	switch (State)
	{
	case EItemState::EIS_Pickup:
		Mesh->SetSimulatePhysics(false);
		Mesh->SetVisibility(true);
		Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Pickup sphere
		PickupRadiusSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		PickupRadiusSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		// Collision box
		Collider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		Collider->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		Collider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case EItemState::EIS_EquipInterping:
		HidePickupWidget();
		Mesh->SetSimulatePhysics(false);
		Mesh->SetVisibility(true);
		Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Pickup sphere
		PickupRadiusSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		PickupRadiusSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Collision box
		Collider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Equipped:
		HidePickupWidget();
		Mesh->SetSimulatePhysics(false);
		Mesh->SetVisibility(true);
		Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Pickup sphere
		PickupRadiusSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		PickupRadiusSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Collision box
		Collider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_PickedUp:
		break;
	case EItemState::EIS_Falling:
		Mesh->SetSimulatePhysics(true);
		Mesh->SetEnableGravity(true);
		Mesh->SetVisibility(true);
		Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// Pickup sphere
		PickupRadiusSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		PickupRadiusSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Collision box
		Collider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_MAX:
		break;
	default:
		break;
	}
}

void AApplePieItem::Interpolate(float DeltaTime)
{
	if (!bInterping)
	{
		return;
	}

	if (Character && InterpZCurve && InterpScaleCurve)
	{
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(InterpTimerHandle);
		// Get curve value corresponding to the elapsed time
		const float CurveValue = InterpZCurve->GetFloatValue(ElapsedTime);
		FVector NewLocation = InterpStartLocation;
		// Calculate location
		const FVector CameraInterpLocation = Character->GetCameraInterpLocation();
		const FVector CurrentLocation = GetActorLocation();
		NewLocation.X = FMath::FInterpTo(CurrentLocation.X, CameraInterpLocation.X, DeltaTime, 30.f);
		NewLocation.Y = FMath::FInterpTo(CurrentLocation.Y, CameraInterpLocation.Y, DeltaTime, 30.f);
		const float InterpHeight = FMath::Abs(InterpStartLocation.Z - CameraInterpLocation.Z);

		NewLocation.Z += CurveValue * InterpHeight;
		SetActorLocation(NewLocation, true, nullptr, ETeleportType::TeleportPhysics);

		// Calculate rotation
		FRotator NewRotation{ GetActorRotation() };
		NewRotation.Yaw = InterpYawOffset + Character->GetFollowCamera()->GetComponentRotation().Yaw;
		SetActorRotation(NewRotation, ETeleportType::TeleportPhysics);

		// Scale
		SetActorScale3D(FVector{ InterpScaleCurve->GetFloatValue(ElapsedTime) });
	}
}

void AApplePieItem::SetState(EItemState InState)
{
	State = InState;
	SetItemProperties();
}

void AApplePieItem::StartInterping(AApplePieCharacter* InCharacter)
{
	if (InCharacter)
	{
		Character = InCharacter;
		InterpStartLocation = GetActorLocation();
		bInterping = true;
		SetState(EItemState::EIS_EquipInterping);
		InterpYawOffset = GetActorRotation().Yaw - Character->GetFollowCamera()->GetComponentRotation().Yaw;
		GetWorldTimerManager().SetTimer(InterpTimerHandle, this, &ThisClass::FinishInterping, InterpZTime);
	}
}

void AApplePieItem::FinishInterping()
{
	bInterping = false;
	if (Character)
	{
		Character->GetPickupItem(this);
	}
	SetActorScale3D(FVector{ 1.f });
}