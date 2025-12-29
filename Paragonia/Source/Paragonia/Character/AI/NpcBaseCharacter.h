// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/PGCharacterBase.h"
#include "Components/StateTreeComponent.h"
#include "NpcBaseCharacter.generated.h"

class UAbilitySystemComponent;
class UCharacterAttributeSet;
class UGameplayAbility;
class UGameplayEffect;

UCLASS()
class PARAGONIA_API ANpcBaseCharacter : public APGCharacterBase
{
	GENERATED_BODY()

public:
	ANpcBaseCharacter();

	virtual void PossessedBy(AController* NewController) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Tick(float DeltaTime) override;

public:
	int32 GetTeamID_Implementation() const override;
	bool GetIsDead_Implementation() const override;

public:
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetTeamId(int32 NewTeamId);

	UFUNCTION(BlueprintCallable, Category = "AI|Target")
	void SetAttackTarget(AActor* NewTarget);

	UFUNCTION(BlueprintPure, Category = "AI|Target")
	AActor* GetAttackTarget() const;

	UFUNCTION(BlueprintPure, Category = "AI")
	float GetAttackRange() const { return AttackRange; };

	UFUNCTION(BlueprintPure, Category = "AI")
	float GetSightRange() const { return SightRange; };

	UFUNCTION(BlueprintPure, Category = "AI")
	bool CanAttack() const;

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetAttackRange(float InRange);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetSightRange(float InRange);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetRotationToTarget(AActor* TargetActor);

protected:
	virtual void BeginPlay() override;

	void GrantStartupAbilities();

	UFUNCTION()
	void OnHealthChanged(float OldValue, float NewValue);

	UFUNCTION()
	virtual void OnRep_TeamId();

	virtual void HandleDeath(AActor* KillerActor);

	UFUNCTION(BlueprintCallable, Category = "Death")
	void StartDeathEffect();

	// 클라 전달용 멀티캐스트 RPC
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HandleDeath();

	UFUNCTION()
	void OnOutOfHealth(AActor* InstigatorActor);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|StateTree")
	TObjectPtr<UStateTreeComponent> StateTreeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Tags")
	FGameplayTag DeadTag;

	UPROPERTY(ReplicatedUsing = OnRep_TeamId, EditAnywhere, BlueprintReadOnly, Category = "AI")
	int32 TeamId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	float AttackRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	float SightRange;

	// 아군에게 보일 외형 (Mesh)
	UPROPERTY(EditDefaultsOnly, Category = "Visual|Team")
	TObjectPtr<USkeletalMesh> AllyMesh;

	// 적군에게 보일 외형 (Mesh)
	UPROPERTY(EditDefaultsOnly, Category = "Visual|Team")
	TObjectPtr<USkeletalMesh> EnemyMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Visual|Team")
	int32 MaterialCounts;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|Target")
	TWeakObjectPtr<AActor> CurrentAttackTarget;

	bool bIsDissolving;
	float DeathAccumulatedTime;

	UPROPERTY(EditDefaultsOnly, Category = "Visual|Dead")
	float DeathDuration;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> DissolveMaterials;

	UPROPERTY(EditAnywhere, Category = "AI")
	int32 RewardGoldAmount = 100;
};
