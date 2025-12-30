#pragma once

#include "CoreMinimal.h"
#include "GA/PGSpawnActorGameplayAbilityBase.h"
#include "Struct/FAttackData.h"
#include "GA_Attack_Sparrow.generated.h"

UCLASS()
class PARAGONIA_API UGA_Attack_Sparrow : public UPGSpawnActorGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_Attack_Sparrow();

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

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	FAttackData AttackData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	FAttackData UltAttackData;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Actor")
	TSubclassOf<AActor> UltBulletClass;

	bool bIsUlt;

};
