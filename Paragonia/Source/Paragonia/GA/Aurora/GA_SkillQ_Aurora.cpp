#include "GA/Aurora/GA_SkillQ_Aurora.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Struct/FAttackData.h"
#include "Animation/PGAnimInstance.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Character/PGPlayerCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayTag/PGGameplayTags.h"

UGA_SkillQ_Aurora::UGA_SkillQ_Aurora()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

void UGA_SkillQ_Aurora::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_SkillQ_Aurora::ActivateAbility - CommitAbility failed"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!IsValid(AttackData.Montage))
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_SkillQ_Aurora::ActivateAbility - Montage is invalid"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UAbilityTask_WaitGameplayEvent* DashStartTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			FGameplayTag::RequestGameplayTag("Event.Aurora.DashStart")
		);

	if (IsValid(DashStartTask))
	{
		DashStartTask->EventReceived.AddDynamic(this, &UGA_SkillQ_Aurora::OnDashStartEvent);
		DashStartTask->ReadyForActivation();
	}

	UAbilityTask_PlayMontageAndWait* Task =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("SkillQ_Task"),
			AttackData.Montage,
			1.0f
		);

	if (IsValid(Task))
	{
		Task->OnCompleted.AddDynamic(this, &UGA_SkillQ_Aurora::OnMontageCompleted);
		Task->OnInterrupted.AddDynamic(this, &UGA_SkillQ_Aurora::OnMontageInterrupted);
		Task->OnCancelled.AddDynamic(this, &UGA_SkillQ_Aurora::OnMontageCancelled);

		Task->ReadyForActivation();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_SkillQ_Aurora::ActivateAbility - Failed to create Ability Task"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}

void UGA_SkillQ_Aurora::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (IsValid(Character))
	{
		Character->GetCharacterMovement()->bUseControllerDesiredRotation = true;

		APlayerController* PC = Cast<APlayerController>(Character->GetController());
		if (IsValid(PC) && PC->IsLocalController())
		{
			PC->SetIgnoreMoveInput(false);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SkillQ_Aurora::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_SkillQ_Aurora::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_SkillQ_Aurora::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_SkillQ_Aurora::OnDashFinished()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!IsValid(Character))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
		return;
	}

	Character->GetCharacterMovement()->StopMovementImmediately();

	if (DashTask)
	{
		DashTask->EndTask();
		DashTask = nullptr;
	}

	ApplyAttackDataOwnerEffects_OnActivate(AttackData);

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_SkillQ_Aurora::OnDashStartEvent(const FGameplayEventData Payload)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!IsValid(Character))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
		return;
	}

	Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;

	AController* Controller = Character->GetController();
	if (IsValid(Controller) && Controller->IsLocalController())
	{
		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			PC->SetIgnoreMoveInput(true);
		}
	}

	const FVector Forward = Character->GetActorForwardVector();
	const FVector StartLocation = Character->GetActorLocation();
	FVector TargetLocation = StartLocation + Forward * DashDistance;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(DashGround), false, Character);

	const FVector TraceStart = TargetLocation + FVector(0, 0, 1000.f);
	const FVector TraceEnd = TargetLocation - FVector(0, 0, 1000.f);

	if (Character->GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		TargetLocation.Z = Hit.Location.Z;
	}

	DashTask = UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(
		this,
		FName("SkillQ_Dash"),
		TargetLocation,
		DashDuration,
		false,
		EMovementMode::MOVE_Walking,
		false,
		nullptr,
		ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity,
		FVector::ZeroVector,
		0.0f
	);

	if (IsValid(DashTask))
	{
		DashTask->OnTimedOut.AddDynamic(this, &UGA_SkillQ_Aurora::OnDashFinished);
		DashTask->OnTimedOutAndDestinationReached.AddDynamic(this, &UGA_SkillQ_Aurora::OnDashFinished);
		DashTask->ReadyForActivation();
	}
}