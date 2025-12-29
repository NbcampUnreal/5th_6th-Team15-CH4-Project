#include "GA/LtBelica/GA_SpawnBullet_LtBelica.h"

#include "Character/PGPlayerCharacterBase.h"
#include "Character/AI/LtBelicaDrone.h"
#include "TargetActor/PGTargetActor.h"
#include "Struct/BulletDataWrapper.h"
#include "Bullet/PGTaskRelatedBullet.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

UGA_SpawnBullet_LtBelica::UGA_SpawnBullet_LtBelica()
{
}

void UGA_SpawnBullet_LtBelica::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthority(&ActivationInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_SpawnBullet_LtBelica::ActivateAbility - No Authority"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_SpawnBullet_LtBelica::ActivateAbility - CommitAbility Failed"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!TriggerEventData)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_SpawnBullet_LtBelica::ActivateAbility - No TriggerEventData"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const UBulletDataWrapper* Wrapper = Cast<UBulletDataWrapper>(TriggerEventData->OptionalObject);
	if (!Wrapper)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_SpawnBullet_LtBelica Ability: Wrapper is null"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CurrentAttackData = Wrapper->Data;
	TSubclassOf<AActor> BulletClass = Wrapper->BulletClass;

	if (!IsValid(BulletClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("BulletClass: BulletClass is not valid"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!IsValid(Character))
	{
		UE_LOG(LogTemp, Warning, TEXT("Character: Character is not valid"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UWorld* World = GetWorld();

	FTransform SpawnTransform = Wrapper->BulletSpawnTransform;
	SpawnTransform.SetRotation(FQuat(Character->GetControlRotation()));

	FActorSpawnParameters Param;
	Param.Owner = GetAvatarActorFromActorInfo();

	AActor* NewBullet = GetWorld()->SpawnActor(BulletClass, &SpawnTransform, Param);
	if (!IsValid(NewBullet))
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_SpawnBullet_Sparrow: NewBullet is not valid"));
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
		return;
	}

	APGTaskRelatedBullet* CreatingBullet = Cast<APGTaskRelatedBullet>(NewBullet);
	if (IsValid(CreatingBullet))
	{
		CreatingBullet->InitBullet(CurrentAttackData);
	}

	ALtBelicaDrone* Drone = Cast<ALtBelicaDrone>(NewBullet);
	if (IsValid(Drone))
	{
		APGPlayerCharacterBase* Player = Cast<APGPlayerCharacterBase>(Character);
		if (IsValid(Player))
		{
			Drone->SetTeamId(IPGTeamStatusInterface::Execute_GetTeamID(Player));
		}
		Drone->InitAttackData(CurrentAttackData);
	}

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGA_SpawnBullet_LtBelica::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
