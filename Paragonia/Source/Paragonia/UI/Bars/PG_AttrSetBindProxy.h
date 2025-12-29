// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PG_AttrSetBindProxy.generated.h"

class UPG_IngameHUD;
class UCharacterAttributeSet;

UENUM(BlueprintType)
enum class EHPBarSlot : uint8
{
    Player,
    Team1,
    Team2,
    OurNexus,
    EnemyNexus,
};

/**
 * 
 */
UCLASS()
class PARAGONIA_API UPG_AttrSetBindProxy : public UObject
{
	GENERATED_BODY()

public:

    void Init(class UPG_IngameHUD* InHUD, EHPBarSlot InSlot, UCharacterAttributeSet* InSet);

    void Unbind();

    UFUNCTION()
    void OnHealth(float OldValue, float NewValue);

    UFUNCTION()
    void OnMaxHealth(float OldValue, float NewValue);

private:

    UPROPERTY() 
    TObjectPtr<UPG_IngameHUD> HUD;

    UPROPERTY() 
    TObjectPtr<UCharacterAttributeSet> BoundSet;

    UPROPERTY()
    EHPBarSlot Slot;
};
