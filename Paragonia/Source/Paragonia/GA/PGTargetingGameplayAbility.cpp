#include "GA/PGTargetingGameplayAbility.h"

#include "Animation/PGRangedAnimInstance.h"
#include "Character/PGPlayerCharacterBase.h"
#include "Struct/FAttackData.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "AbilitySystemComponent.h"

UPGTargetingGameplayAbility::UPGTargetingGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	RelativeName = "";

	bUseHitResult = true;
}

void UPGTargetingGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!IsValid(AttackData.Montage))
	{
		UE_LOG(LogTemp, Warning, TEXT("UPGTargetingGameplayAbility::ActivateAbility - No Montage"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* Task =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TaskName,
			AttackData.Montage,
			1.0f,
			TargetingStartMontageSectionName
		);

	if (IsValid(Task))
	{
		Task->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
		Task->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);

		Task->ReadyForActivation();

		MontageTask = Task;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UPGTargetingGameplayAbility::ActivateAbility - Failed to create Ability Task"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_WaitInputPress* InputPressTask =
		UAbilityTask_WaitInputPress::WaitInputPress(this);

	InputPressTask->OnPress.AddDynamic(this, &ThisClass::OnInputPressed);
	InputPressTask->ReadyForActivation();
}

void UPGTargetingGameplayAbility::OnHitResultEvent(const FGameplayEventData Payload)
{
	ApplyAttackDataEffects_OnHit(AttackData, Payload);
}

void UPGTargetingGameplayAbility::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UPGTargetingGameplayAbility::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UPGTargetingGameplayAbility::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UPGTargetingGameplayAbility::OnInputPressed(float InTimeWaited)
{
	if (!IsValid(MontageTask))
	{
		UE_LOG(LogTemp, Warning, TEXT("UPGTargetingGameplayAbility::OnInputPressed - MontageTask is invalid"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetCurrentActorInfo()->AvatarActor.Get());
	if (IsValid(Character))
	{
		if (UAnimInstance* AI = Character->GetMesh()->GetAnimInstance())
		{
			if (auto PGAnimInstance = Cast<UPGRangedAnimInstance>(AI))
			{
				PGAnimInstance->SetCurrentAttackData(AttackData);
				PGAnimInstance->SetBulletClass(SpawnActorClass);
				PGAnimInstance->SetTimeWaited(InTimeWaited);

				if (bIsUseSocket)
				{
					UMeshComponent* Mesh = Cast<ACharacter>(GetAvatarActorFromActorInfo())->GetMesh();
					if (IsValid(Mesh))
					{
						PGAnimInstance->SetBulletSpawnTransform(Mesh->GetSocketTransform(SocketName));
					}
				}
				else if (IsValid(TransformClass))
				{
					TArray<UActorComponent*> OutComps;
					GetAvatarActorFromActorInfo()->GetComponents(OutComps);
					for (UActorComponent* Comp : OutComps)
					{
						if (IsValid(Comp))
						{
							if (Comp->IsA(TransformClass) && Comp->GetFName().ToString().Contains(RelativeName))
							{
								USceneComponent* SceneComp = Cast<USceneComponent>(Comp);
								if (IsValid(SceneComp))
								{
									PGAnimInstance->SetBulletSpawnTransform(SceneComp->GetComponentTransform());
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	MontageTask->EndTask();

	MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TaskName,
			AttackData.Montage,
			1.0f,
			TargetingEndtMontageSectionName
		);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);

	MontageTask->ReadyForActivation();

	if (bUseHitResult)
	{
		UAbilityTask_WaitGameplayEvent* HitResultTask =
			UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AttackData.HitResultTag);

		if (IsValid(HitResultTask))
		{
			HitResultTask->EventReceived.AddDynamic(this, &ThisClass::OnHitResultEvent);
			HitResultTask->ReadyForActivation();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UPGTargetingGameplayAbility::ActivateAbility - Failed to create HitResult Ability Task"));
		}

		if (EffectActorClass)
		{
			CreateEffectActor();
		}
	}
}

void UPGTargetingGameplayAbility::CreateEffectActor()
{
}
