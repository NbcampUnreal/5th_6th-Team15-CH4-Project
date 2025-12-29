// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "WaveDefinition.generated.h"

USTRUCT(BlueprintType)
struct FWaveSpawnInfo
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly)
    FGameplayTag SpawnNpcTag;

    UPROPERTY(EditDefaultsOnly)
    int32 SpawnCount = 0;

    UPROPERTY(EditDefaultsOnly)
    int32 StartSpawnTime = 0;
};

/**
 * 
 */
UCLASS()
class PARAGONIA_API UWaveDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditDefaultsOnly, Category = "Wave")
    TArray<FWaveSpawnInfo> WaveSpawnList;

    UPROPERTY(EditDefaultsOnly, Category = "Wave")
    float SpawnInterval = 1.0f;
};
