#include "PGShopComponent.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "PGShopCatalog.h"
#include "PlayerState/PGPlayerState.h" 
#include "Inventory/PGInventoryComponent.h"

UPGShopComponent::UPGShopComponent()
{
    SetIsReplicatedByDefault(true);
}

UDataTable* UPGShopComponent::GetItemDataTable() const
{
    return ItemDataTable.IsNull() ? nullptr : ItemDataTable.LoadSynchronous();
}

void UPGShopComponent::BeginPlay()
{
    Super::BeginPlay();

    if (GetOwnerRole() == ROLE_Authority)
    {
        UDataTable* DT = GetItemDataTable();
        if (!DT)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Shop] BeginPlay: ItemDataTable is null"));
            return;
        }
    }
}

void UPGShopComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

bool UPGShopComponent::FindItem(FName ItemId, FPGShopItemRow& OutItem) const
{
    UDataTable* DT = GetItemDataTable();
    if (!DT)
        return false;

    static const FString Context(TEXT("ShopFindItem"));

    // 1) RowName == ItemId 인 경우
    if (const FPGShopItemRow* Row = DT->FindRow<FPGShopItemRow>(ItemId, Context))
    {
        OutItem = *Row;
        // Row 안의 ItemId가 비어있으면 RowName으로 보정
        if (OutItem.ItemId.IsNone())
        {
            OutItem.ItemId = ItemId;
        }
        return true;
    }

    // RowName이 다르더라도 Row.ItemId로 찾을 수 있게
    const TMap<FName, uint8*>& RowMap = DT->GetRowMap();
    for (const auto& Pair : RowMap)
    {
        const FPGShopItemRow* Row = reinterpret_cast<const FPGShopItemRow*>(Pair.Value);
        if (Row && Row->ItemId == ItemId)
        {
            OutItem = *Row;
            return true;
        }
    }

    return false;
}

int32 UPGShopComponent::GetRemainingStock(FName ItemId) const
{
    return -1;
}

void UPGShopComponent::RequestBuy(FName ItemId)
{
    if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
    {
        if (PC->IsLocalController())
        {
            ServerBuy(ItemId);
        }
    }
}

void UPGShopComponent::RequestSell(int32 Index)
{
    if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
    {
        if (PC->IsLocalController())
        {
            ServerSell(Index);
        }
    }
}

void UPGShopComponent::ServerBuy_Implementation(FName ItemId)
{
    FPGShopItemRow Item;
    if (!FindItem(ItemId, Item))
    {
        ClientBuyResult(EShopBuyResult::InvalidItem, ItemId);
        return;
    }

    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC)
    {
        ClientBuyResult(EShopBuyResult::InvalidItem, ItemId);
        return;
    }

    APGPlayerState* PS = PC->GetPlayerState<APGPlayerState>();
    if (!PS)
    {
        ClientBuyResult(EShopBuyResult::InvalidItem, ItemId);
        return;
    }

    const int32 TotalCost = Item.Price;

    if (!PS->CanAfford(TotalCost))
    {
        ClientBuyResult(EShopBuyResult::NotEnoughGold, ItemId);
        return;
    }

    PS->AddGold(-TotalCost);

    GrantItemToOwner(Item);

    ClientBuyResult(EShopBuyResult::Success, ItemId);
}

void UPGShopComponent::ServerSell_Implementation(int32 Index)
{
    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC)
    {
        return;
    }

    APGPlayerState* PS = PC->GetPlayerState<APGPlayerState>();
    if (!PS)
    {
        return;
    }

    UPGInventoryComponent* Inv = PS->FindComponentByClass<UPGInventoryComponent>();
    if (!Inv)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shop] GrantItemToOwner: InventoryComponent not found on PlayerState"));
        return;
    }

    FPGShopItemRow Item;
    FName ItemId = Inv->Slots[Index].ItemId;
    if (!FindItem(ItemId, Item))
    {
        return;
    }

    const int32 TotalCost = Item.Price * 7 / 10;

    PS->AddGold(TotalCost);

	Inv->RemoveItem(Index);

    ClientBuyResult(EShopBuyResult::Success, ItemId);
}

void UPGShopComponent::ClientBuyResult_Implementation(EShopBuyResult Result, FName ItemId)
{
    OnBuyResult.Broadcast(Result, ItemId);
}

void UPGShopComponent::GrantItemToOwner(const FPGShopItemRow& Item)
{
    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shop] GrantItemToOwner: Owner is not PlayerController"));
        return;
    }

    APGPlayerState* PS = Cast<APGPlayerState>(PC->PlayerState);
    if (!PS)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shop] GrantItemToOwner: PlayerState is null"));
        return;
    }

    UPGInventoryComponent* Inv = PS->FindComponentByClass<UPGInventoryComponent>();
    if (!Inv)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shop] GrantItemToOwner: InventoryComponent not found on PlayerState"));
        return;
    }

    if(Item.Category == EShopCategory::Consumable)
    {
		Inv->ApplyAllItemStatsToOwner(Item);
	}
    else
    {
        const bool bOk = Inv->AddItem(Item.ItemId);

        if (!bOk)
        {
            PS->AddGold(Item.Price);
        }
    }
}
