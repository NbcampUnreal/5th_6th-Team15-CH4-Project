#include "Bullet/PGNormalBullet_Sparrow.h"

#include "Bullet/PGMultiBulletCreator.h"
#include "GA/Sparrow/GA_SpawnBullet_Sparrow.h"
#include "Interface/PGTeamStatusInterface.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"

APGNormalBullet_Sparrow::APGNormalBullet_Sparrow()
	: APGCreateTargetActorBullet()
{
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(SceneRoot);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(Sphere);

	StaticMesh->SetRelativeLocation(FVector(-50.0f, 0.0f, 0.0f));
	StaticMesh->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));

	Projectile = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	Projectile->InitialSpeed = 2000.0f;
	Projectile->Velocity = FVector(1.0f, 0.0f, 0.2f);
	Projectile->bRotationFollowsVelocity = true;
}

void APGNormalBullet_Sparrow::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBeginOverlap);
}

void APGNormalBullet_Sparrow::OnBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!HasAuthority()) return;

	if (!IsValid(Owner))
	{
		UE_LOG(LogTemp, Warning, TEXT("APGNormalBullet_Sparrow::OnBeginOverlap - Not Valid Owner"));
		Destroy();
	}
	else if (!IsValid(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("APGNormalBullet_Sparrow::OnBeginOverlap - Not Valid OtherActor"));
		Destroy();
	}
	else if (Owner == OtherActor ||
			 Owner == OtherActor->GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("APGNormalBullet_Sparrow::OnBeginOverlap - Not Available Target"));
	}
	else
	{
		if (OtherActor->GetClass()->ImplementsInterface(UPGTeamStatusInterface::StaticClass()))
		{
			int32 OwnerTeamID = IPGTeamStatusInterface::Execute_GetTeamID(Owner);
			int32 TargetTeamID = IPGTeamStatusInterface::Execute_GetTeamID(OtherActor);

			if (OwnerTeamID != TargetTeamID)
			{
				HitCheckNotify();
				if (!bIsPierce)
				{
					Destroy();
				}
			}
		}
		else
		{
			for (auto& Ignore : OverlapIgnoreClasses)
			{
				if (IsValid(Ignore))
				{
					if (OtherActor->IsA(Ignore))
					{
						return;
					}
				}
			}

			Destroy();
		}
	}
}
