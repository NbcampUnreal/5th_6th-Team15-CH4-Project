#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "PGInventoryComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInventoryChanged);

struct FPGShopItemRow;

USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    bool bEmpty = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    FName ItemId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 Count = 0;

    UPROPERTY()
    FActiveGameplayEffectHandle AppliedEffectHandle;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PARAGONIA_API UPGInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPGInventoryComponent();

    UFUNCTION(BlueprintCallable)
    void RequestSwapSlot(int32 FromIndex, int32 ToIndex);

    UFUNCTION(BlueprintCallable)
    bool AddItem(FName ItemId);

    UFUNCTION(BlueprintCallable)
    bool RemoveItem(int32 Index);

    UFUNCTION(BlueprintCallable)
    const FInventorySlot& GetSlot(int32 Index) const;

    UFUNCTION(BlueprintCallable)
    UDataTable* GetItemDataTable() const;

    FActiveGameplayEffectHandle ApplyAllItemStatsToOwner(const FPGShopItemRow& Item);
protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** 서버 슬롯 교환 */
    UFUNCTION(Server, Reliable)
    void ServerSwapSlot(int32 FromIndex, int32 ToIndex);

    /** 서버 아이템 추가 */
    UFUNCTION(Server, Reliable)
    void ServerAddItem(FName ItemId);

    UFUNCTION(Server, Reliable)
    void ServerRemoveItem(int32 Index);

    UFUNCTION()
    void OnRep_Inventory();

    FActiveGameplayEffectHandle ApplyItemStatsToOwner(FName ItemId);


public:
    FOnInventoryChanged OnInventoryChanged;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
    TSoftObjectPtr<UDataTable> ItemDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 MaxSlots = 6;

    UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Inventory)
    TArray<FInventorySlot> Slots;
};
