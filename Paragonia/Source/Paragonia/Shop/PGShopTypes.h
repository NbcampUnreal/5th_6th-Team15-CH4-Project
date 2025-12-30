// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "PGShopTypes.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
    Common      UMETA(DisplayName = "Common"),
    Uncommon    UMETA(DisplayName = "Uncommon"),
    Rare        UMETA(DisplayName = "Rare"),
    Unique      UMETA(DisplayName = "Unique"),
    Legendary   UMETA(DisplayName = "Legendary")
};

UENUM(BlueprintType)
enum class EShopCategory : uint8
{
    All     UMETA(DisplayName = "All"),
    Weapon  UMETA(DisplayName = "Weapon"),
    Armor   UMETA(DisplayName = "Armor"),
    Consumable UMETA(DisplayName = "Consumable"),
    Etc     UMETA(DisplayName = "Etc"),
};

USTRUCT(BlueprintType)
struct FPGShopItemRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Price = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EShopCategory Category = EShopCategory::All;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EItemRarity Rarity = EItemRarity::Common;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag ItemTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bLimited"))
    int32 Stock = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stats")
    TMap<FGameplayTag, float> ItemStats; 

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stats")
    TSubclassOf<class UGameplayEffect> EquipmentGE;
};