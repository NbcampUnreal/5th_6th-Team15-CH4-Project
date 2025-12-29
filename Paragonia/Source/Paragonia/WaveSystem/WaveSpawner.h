// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "Struct/NpcDataRow.h"
#include "WaveDefinition.h"
#include "WaveSpawner.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;
class APGGameStateBase;

UCLASS()
class PARAGONIA_API AWaveSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AWaveSpawner();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION()
	void OnWaveStart();

	UFUNCTION()
	void HandleGameTimeUpdate(int32 CurrentTime);

protected:
	void CacheDataTableTags();

	void HandleSpawnTick();

	void SpawnMinion(FName RowName);

	APGGameStateBase* GetNowWorldGameState();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USplineComponent> PathSpline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Config")
	TObjectPtr<UWaveDefinition> CurrentWaveDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Config")
	TObjectPtr<UDataTable> WaveDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|GAS")
	TSubclassOf<UGameplayEffect> MinionInitEffectClass;

	// 이 Spawner에서 생성될 유닛의 TeamID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Config")
	uint8 TeamId;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Config")
	uint8 SpawnRate;

private:
	TMap<FGameplayTag, FName> TagToRowMap;

	TArray<FName> SpawnNames;
	int32 CurrentSpawnIndex;

	FTimerHandle SpawnTimerHandle;
};
