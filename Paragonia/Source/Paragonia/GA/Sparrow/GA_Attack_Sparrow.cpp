#include "GA/Sparrow/GA_Attack_Sparrow.h"

#include "Animation/PGRangedAnimInstance.h"
#include "Character/PGPlayerCharacterBase.h"
#include "Struct/FAttackData.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"

UGA_Attack_Sparrow::UGA_Attack_Sparrow()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	bReplicateInputDirectly = true;
}

void UGA_Attack_Sparrow::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = GetAvatarActorFromActorInfo()->GetComponentByClass<UAbilitySystemComponent>();
	if (!IsValid(ASC))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bIsUlt = ASC->GetOwnedGameplayTags().HasTag(FGameplayTag::RequestGameplayTag(TEXT("Character.State.SparrowUlt")));

	FAttackData& RealAttackData = bIsUlt ? UltAttackData : AttackData;
	TSubclassOf<AActor> RealBulletClass = bIsUlt ? UltBulletClass : SpawnActorClass;

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (IsValid(Character))
	{
		if (UAnimInstance* AI = Character->GetMesh()->GetAnimInstance())
		{
			if (auto PGAnimInstance = Cast<UPGRangedAnimInstance>(AI))
			{
				PGAnimInstance->SetCurrentAttackData(RealAttackData);
				PGAnimInstance->SetBulletClass(RealBulletClass);
				PGAnimInstance->SetBulletSpawnTransform(Character->GetMesh()->GetSocketTransform(FName(TEXT("BowEmitterSocket"))));
			}
		}
	}

	if (!IsValid(RealAttackData.Montage))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* Task =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("AttackTask"),
			RealAttackData.Montage,
			1.0f
		);

	if (IsValid(Task))
	{
		Task->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
		Task->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
		Task->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);

		Task->ReadyForActivation();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_Attack_Sparrow::ActivateAbility - Failed to create Ability Task"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}

	UAbilityTask_WaitGameplayEvent* HitResultTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, RealAttackData.HitResultTag);

	if (IsValid(HitResultTask))
	{
		HitResultTask->EventReceived.AddDynamic(this, &ThisClass::OnHitResultEvent);
		HitResultTask->ReadyForActivation();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_Attack_Sparrow::ActivateAbility - Failed to create HitResult Ability Task"));
	}
}

void UGA_Attack_Sparrow::OnHitResultEvent(const FGameplayEventData Payload)
{
	ApplyAttackDataEffects_OnHit(AttackData, Payload);
}

void UGA_Attack_Sparrow::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_Attack_Sparrow::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UGA_Attack_Sparrow::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}
