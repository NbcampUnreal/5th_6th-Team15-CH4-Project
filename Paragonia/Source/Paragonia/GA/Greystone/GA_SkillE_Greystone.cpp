#include "GA/Greystone/GA_SkillE_Greystone.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Struct/FAttackData.h"
#include "Struct/AttackDataWrapper.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Animation/PGAnimInstance.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"

UGA_SkillE_Greystone::UGA_SkillE_Greystone()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

void UGA_SkillE_Greystone::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_SkillE_Greystone::ActivateAbility - CommitAbility failed"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!IsValid(AttackData.Montage))
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_SkillE_Greystone::ActivateAbility - Invalid AttackData Montage"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (IsValid(ASC))
	{
		ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Attaking")));
	}

	if (HasAuthority(&ActivationInfo))
	{
		UAbilityTask_WaitGameplayEvent* HitResultTask =
			UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
				this,
				AttackData.HitResultTag
			);
		if (IsValid(HitResultTask))
		{
			HitResultTask->EventReceived.AddDynamic(this, &UGA_SkillE_Greystone::OnHitResultEvent);
			HitResultTask->ReadyForActivation();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UGA_SkillE_Greystone::ActivateAbility - Failed to create HitResultTask"));
		}

		StartAura();
	}
	
	UAbilityTask_PlayMontageAndWait* Task =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("SkillE_Task"),
			AttackData.Montage,
			1.0f
		);
	if (IsValid(Task))
	{
		Task->OnCompleted.AddDynamic(this, &UGA_SkillE_Greystone::OnMontageCompleted);
		Task->OnInterrupted.AddDynamic(this, &UGA_SkillE_Greystone::OnMontageInterrupted);
		Task->OnCancelled.AddDynamic(this, &UGA_SkillE_Greystone::OnMontageCancelled);

		Task->ReadyForActivation();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_SkillE_Greystone::ActivateAbility - Failed to create Ability Task"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ApplyAttackDataOwnerEffects_OnActivate(AttackData);
}

void UGA_SkillE_Greystone::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	if (HasAuthority(&ActivationInfo))
	{
		StopAura();
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (IsValid(ASC))
	{
		ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Attaking")));
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SkillE_Greystone::OnHitResultEvent(const FGameplayEventData Payload)
{
	ApplyAttackDataEffects_OnHit(AttackData, Payload);
}

void UGA_SkillE_Greystone::OnMontageCompleted()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (IsValid(ASC))
	{
		ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Attaking")));
	}
}

void UGA_SkillE_Greystone::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_SkillE_Greystone::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_SkillE_Greystone::StartAura()
{
	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		World->GetTimerManager().ClearTimer(AuraEndTimerHandle);
		World->GetTimerManager().ClearTimer(HitCheckTimerHandle);

		World->GetTimerManager().SetTimer(
			AuraEndTimerHandle,
			this,
			&UGA_SkillE_Greystone::OnAuraDurationFinished,
			AuraLifeTime,
			false
		);

		World->GetTimerManager().SetTimer(
			HitCheckTimerHandle,
			this,
			&UGA_SkillE_Greystone::TickHitCheck,
			AuraPeriod,
			true
		);
	}
}

void UGA_SkillE_Greystone::StopAura()
{
	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		World->GetTimerManager().ClearTimer(AuraEndTimerHandle);
		World->GetTimerManager().ClearTimer(HitCheckTimerHandle);
	}
}

void UGA_SkillE_Greystone::TickHitCheck()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!IsValid(Avatar))
	{
		return;
	}

	UAttackDataWrapper* Wrapper = NewObject<UAttackDataWrapper>(this);
	Wrapper->Data = AttackData;

	FGameplayEventData EventData;
	EventData.Instigator = Avatar;
	EventData.OptionalObject = Wrapper;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Avatar,
		FGameplayTag::RequestGameplayTag(FName("Event.Character.HitCheck")),
		EventData
	);
}

void UGA_SkillE_Greystone::OnAuraDurationFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
