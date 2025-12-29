#include "Bullet/PGCreateTargetActorBullet.h"

#include "Character/PGPlayerCharacterBase.h"
#include "Character/AI/LtBelicaDrone.h"
#include "TargetActor/PGRangedTargetActor.h"
#include "Struct/AttackDataWrapper.h"

#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

APGCreateTargetActorBullet::APGCreateTargetActorBullet()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APGCreateTargetActorBullet::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		SetReplicates(true);
		SetReplicateMovement(true);
	}
}

void APGCreateTargetActorBullet::HitCheckNotify()
{
	if (!IsValid(Owner))
	{
		return;
	}

	APGPlayerCharacterBase* PlayerCharacter = Cast<APGPlayerCharacterBase>(Owner);
	if (IsValid(PlayerCharacter))
	{
		UAttackDataWrapper* Wrapper = NewObject<UAttackDataWrapper>(this);
		Wrapper->Data = AttackData;

		FGameplayEventData EventData;
		EventData.Instigator = Owner;
		EventData.OptionalObject = Wrapper;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			Owner,
			FGameplayTag::RequestGameplayTag(FName("Event.Character.HitCheck")),
			EventData
		);
	}

	ALtBelicaDrone* Drone = Cast<ALtBelicaDrone>(Owner);
	if (IsValid(Drone))
	{
		UAttackDataWrapper* Wrapper = NewObject<UAttackDataWrapper>(this);
		Wrapper->Data = AttackData;

		FGameplayEventData EventData;
		EventData.Instigator = Owner;
		EventData.OptionalObject = Wrapper;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			Owner,
			FGameplayTag::RequestGameplayTag(FName("Event.Character.HitCheck")),
			EventData
		);
	}
}
