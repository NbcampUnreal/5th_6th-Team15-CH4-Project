#include "GA/Npc/GA_DroneHitResult.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UGA_DroneHitResult::UGA_DroneHitResult()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;
}

void UGA_DroneHitResult::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_WaitGameplayEvent* HitResultTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AttackData.HitResultTag);

	if (IsValid(HitResultTask))
	{
		HitResultTask->EventReceived.AddDynamic(this, &ThisClass::OnHitResultEvent);
		HitResultTask->ReadyForActivation();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_DroneHitResult::ActivateAbility - Failed to create HitResult Ability Task"));
	}
}

void UGA_DroneHitResult::OnHitResultEvent(const FGameplayEventData Payload)
{
	ApplyAttackDataEffects_OnHit(AttackData, Payload);
}
