#include "TargetActor/PGTargetActor.h"
#include "Abilities/GameplayAbility.h"
#include "DrawDebugHelpers.h"
#include "Character/PGPlayerCharacterBase.h"
#include "Struct/FAttackData.h"
#include "Components/CapsuleComponent.h"

APGTargetActor::APGTargetActor()
{
	ShouldProduceTargetDataOnServer = true;
}

void APGTargetActor::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
}

void APGTargetActor::ConfirmTargetingAndContinue()
{
	check(ShouldProduceTargetData());
	if (IsConfirmTargetingAllowed())
	{
		TArray<FHitResult> OutHitResults;
		bool bHit = PerformTrace(OutHitResults);

		FGameplayAbilityTargetDataHandle DataHandle;
		if (bHit)
		{
			TSet<AActor*> UniqueActors;

			for (const FHitResult& Hit : OutHitResults)
			{
				AActor* HitActor = Hit.GetActor();
				if (!IsValid(HitActor) || UniqueActors.Contains(HitActor))
				{
					continue;
				}

				UniqueActors.Add(HitActor);
				DataHandle.Add(new FGameplayAbilityTargetData_SingleTargetHit(Hit));
			}
		}

		TargetDataReadyDelegate.Broadcast(DataHandle);
	}
}

void APGTargetActor::SetAttackData(const FAttackData& InAttackData)
{
	AttackData = InAttackData;
}

bool APGTargetActor::PerformTrace(TArray<FHitResult>& OutHits) const
{
	switch (AttackData.SweepShape)
	{
	case EPGAttackShape::Sphere:
		return SphereTrace(OutHits);

	case EPGAttackShape::Capsule:
		return CapsuleTrace(OutHits);

	case EPGAttackShape::Box:
		return BoxTrace(OutHits);

	default:
		return false;
	}
}

bool APGTargetActor::SphereTrace(TArray<FHitResult>& OutHitResults) const
{
	FVector Start = SourceActor->GetActorLocation();
	FVector End = Start + SourceActor->GetActorForwardVector() * AttackData.Range;

	FCollisionQueryParams Params(NAME_None, false, SourceActor);

	bool bHit = SourceActor->GetWorld()->SweepMultiByChannel(
		OutHitResults,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(AttackData.Radius),
		Params
	);

	return bHit;
}

bool APGTargetActor::CapsuleTrace(TArray<FHitResult>& OutHitResults) const
{
	FVector Start = SourceActor->GetActorLocation();
	FVector End = Start + SourceActor->GetActorForwardVector() * AttackData.Range;
	FCollisionQueryParams Params(NAME_None, false, SourceActor);

	ACharacter* Character = Cast<ACharacter>(SourceActor);
	float HalfHeight = 88.f;

	if (Character)
	{
		UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
		if (Capsule)
		{
			HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}
	}

	bool bHit = SourceActor->GetWorld()->SweepMultiByChannel(
		OutHitResults,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeCapsule(AttackData.Radius, HalfHeight),
		Params
	);

	return bHit;
}

bool APGTargetActor::BoxTrace(TArray<FHitResult>& OutHitResults) const
{
	FVector Start = SourceActor->GetActorLocation();
	FVector End = Start + SourceActor->GetActorForwardVector() * AttackData.Range;

	FCollisionQueryParams Params(NAME_None, false, SourceActor);

	ACharacter* Character = Cast<ACharacter>(SourceActor);
	float HalfHeight = 88.f;

	if (Character)
	{
		UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
		if (Capsule)
		{
			HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}
	}

	FVector BoxExtent;
	BoxExtent.X = AttackData.Range * 0.5f;
	BoxExtent.Y = AttackData.Radius;
	BoxExtent.Z = HalfHeight;

	bool bHit = SourceActor->GetWorld()->SweepMultiByChannel(
		OutHitResults,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeBox(BoxExtent),
		Params
	);

	return bHit;
}
