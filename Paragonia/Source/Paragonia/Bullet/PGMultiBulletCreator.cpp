#include "Bullet/PGMultiBulletCreator.h"

#include "Bullet/PGNormalBullet_Sparrow.h"

#include "Camera/CameraComponent.h"

#include "Abilities/GameplayAbility.h"

APGMultiBulletCreator::APGMultiBulletCreator()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APGMultiBulletCreator::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimerForNextTick(
			[this]()
			{
				CreateBullets();
				Destroy();
			}
		);
	}
}

void APGMultiBulletCreator::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		SetReplicates(false);
	}
}

void APGMultiBulletCreator::CreateBullets()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(BulletClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("APGMultiBulletCreator::CreateBullets - Invalid Bullet Class"));
		return;
	}

	FTransform OriginTransform = GetActorTransform();
	float StartAngle = (BulletCount % 2 == 1 ? -AngleSpacing * (BulletCount / 2) : -AngleSpacing * (BulletCount / 2) - (AngleSpacing / 2.0f));
	for (int i = 0; i < BulletCount; ++i)
	{
		FTransform SpawnTransform = OriginTransform;
		FQuat DeltaRot = FQuat(FRotator(0.0f, StartAngle + AngleSpacing * i, 0.0f));
		SpawnTransform.SetRotation(SpawnTransform.GetRotation() * DeltaRot);
		FActorSpawnParameters Param;
		Param.Owner = Owner;
		AActor* NewBullet = GetWorld()->SpawnActor(BulletClass, &SpawnTransform, Param);
		if (!IsValid(NewBullet)) continue;

		APGTaskRelatedBullet* CreatingBullet = Cast<APGTaskRelatedBullet>(NewBullet);
		if (IsValid(CreatingBullet))
		{
			CreatingBullet->InitBullet(AttackData);
		}
	}

}

