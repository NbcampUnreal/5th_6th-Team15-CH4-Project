#pragma once

#include "CoreMinimal.h"
#include "GA/PGSpawnActorGameplayAbilityBase.h"
#include "Struct/FAttackData.h"
#include "PGTargetingGameplayAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;

UCLASS()
class PARAGONIA_API UPGTargetingGameplayAbility : public UPGSpawnActorGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UPGTargetingGameplayAbility();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

private:
	UFUNCTION()
	void OnHitResultEvent(const FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnInputPressed(float InTimeWaited);

protected:
	virtual void CreateEffectActor();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	FAttackData AttackData;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Task")
	FName TaskName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Task")
	FName TargetingStartMontageSectionName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Task")
	FName TargetingEndtMontageSectionName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Task")
	bool bUseHitResult;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spawn")
	bool bIsUseSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spawn", meta = (EditCondition = "bIsUseSocket", EditConditionHides))
	FName SocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spawn", meta = (EditCondition = "!bIsUseSocket", EditConditionHides))
	TSubclassOf<UActorComponent> TransformClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spawn", meta = (EditCondition = "!bIsUseSocket", EditConditionHides))
	FString RelativeName;
	FTransform SpawnTransform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spawn")
	TSubclassOf<AActor> EffectActorClass;

	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;
};
