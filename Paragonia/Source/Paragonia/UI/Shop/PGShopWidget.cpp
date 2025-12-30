#include "UI/Shop/PGShopWidget.h"

#include "Components/TileView.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

#include "Shop/PGShopComponent.h"
#include "Shop/PGShopCatalog.h"   
#include "Shop/PGShopItemObject.h"
#include "UI/Shop/PGShopDetailWidget.h"
#include "UI/Inventory/PGInventoryWidget.h"
#include "Inventory/PGInventoryComponent.h"
#include "PlayerState/PGPlayerState.h"

void UPGShopWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ItemTileView)
    {
        ItemTileView->SetSelectionMode(ESelectionMode::Single);

        ItemTileView->OnItemSelectionChanged().AddUObject(this, &UPGShopWidget::HandleItemClicked);
    }
}

void UPGShopWidget::InitWithShopComponent(UPGShopComponent* InShop)
{
    ShopComp = InShop;

    if (ShopComp)
    {
        ShopComp->OnBuyResult.AddDynamic(this, &UPGShopWidget::OnBuyResult);
    }

    APGPlayerState* PS = Cast<APGPlayerState>(GetOwningPlayerState());
    if (IsValid(PS))
    {
        PS->OnGoldChanged.AddUObject(this, &UPGShopWidget::HandleGoldChange);
    }

    BuildCategoryButtons();
    RebuildList();
    HandleGoldChange(PS->GetGold());

    if (PlayerInventoryWidget)
    {
        PlayerInventoryWidget->InitFromOwningPlayer();
        PlayerInventoryWidget->OnInventorySlotSelected.AddUObject(this, &UPGShopWidget::HandleInventorySlotSelected);
        PlayerInventoryWidget->OnInventorySlotRightClick.AddUObject(this, &UPGShopWidget::HandleInventorySlotRightClick);
    }
}

void UPGShopWidget::BuildCategoryButtons()
{
    if (!CategoryScroll)
    {
        return;
    }

    CategoryScroll->ClearChildren();

    auto CreateBtn = [&](const TCHAR* Label)
        {
            UButton* Btn = NewObject<UButton>(this);
            UTextBlock* Txt = NewObject<UTextBlock>(Btn);
            Txt->SetText(FText::FromString(Label));
            Btn->AddChild(Txt);
            CategoryScroll->AddChild(Btn);
            return Btn;
        };

    {
        UButton* Btn = CreateBtn(TEXT("전체"));
        Btn->OnClicked.AddDynamic(this, &UPGShopWidget::OnCategoryAll);
    }
    {
        UButton* Btn = CreateBtn(TEXT("무기"));
        Btn->OnClicked.AddDynamic(this, &UPGShopWidget::OnCategoryWeapon);
    }
    {
        UButton* Btn = CreateBtn(TEXT("방어구"));
        Btn->OnClicked.AddDynamic(this, &UPGShopWidget::OnCategoryArmor);
    }
    {
        UButton* Btn = CreateBtn(TEXT("소비"));
        Btn->OnClicked.AddDynamic(this, &UPGShopWidget::OnCategoryConsumable);
    }
    {
        UButton* Btn = CreateBtn(TEXT("기타"));
        Btn->OnClicked.AddDynamic(this, &UPGShopWidget::OnCategoryEtc);
    }
}

void UPGShopWidget::OnCategoryAll() { CurrentCategory = EShopCategory::All;        RebuildList(); }
void UPGShopWidget::OnCategoryWeapon() { CurrentCategory = EShopCategory::Weapon;     RebuildList(); }
void UPGShopWidget::OnCategoryArmor() { CurrentCategory = EShopCategory::Armor;      RebuildList(); }
void UPGShopWidget::OnCategoryConsumable() { CurrentCategory = EShopCategory::Consumable; RebuildList(); }
void UPGShopWidget::OnCategoryEtc() { CurrentCategory = EShopCategory::Etc;        RebuildList(); }

void UPGShopWidget::HandleInventorySlotSelected(int32 SlotIndex)
{
    if (!DetailWidget)
    {
        return;
    }

    DetailWidget->SetCurrentSlotIndex(SlotIndex);
}

void UPGShopWidget::HandleInventorySlotRightClick(int32 SlotIndex)
{
    if (!ShopComp)
    {
        return;
    }

    ShopComp->RequestSell(SlotIndex);
}

void UPGShopWidget::HandleGoldChange(int32 NewGold)
{
    if (!GoldText)
    {
        return;
    }

    GoldText->SetText(
        FText::FromString(
            FString::Printf(TEXT("Gold: %d"), NewGold)
        )
    );
}

void UPGShopWidget::RebuildList()
{
    if (!ItemTileView || !ShopComp)
    {
        return;
    }

    ItemTileView->ClearListItems();

    UDataTable* DT = ShopComp->GetItemDataTable();
    if (!DT)
    {
        return;
    }

    const TMap<FName, uint8*>& RowMap = DT->GetRowMap();
    for (const auto& Pair : RowMap)
    {
        const FPGShopItemRow* RowPtr = reinterpret_cast<const FPGShopItemRow*>(Pair.Value);
        if (!RowPtr)
        {
            continue;
        }

        FPGShopItemRow Row = *RowPtr;

        if (Row.ItemId.IsNone())
        {
            Row.ItemId = Pair.Key;
        }

        if (CurrentCategory != EShopCategory::All && Row.Category != CurrentCategory)
        {
            continue;
        }

        UPGShopItemObject* Obj = NewObject<UPGShopItemObject>(this);
        Obj->Data = Row;
        ItemTileView->AddItem(Obj);
    }
}

void UPGShopWidget::HandleItemClicked(UObject* Item)
{
    if (!DetailWidget) 
    {
        return;
    }

    UPGShopItemObject* Obj = Cast<UPGShopItemObject>(Item);
    if (!Obj) 
    {
        return;
    }

    if (ItemTileView->IsItemSelected(Item))
    {
        DetailWidget->BindShop(ShopComp);
        DetailWidget->SetItem(Obj->Data);
    }
}

void UPGShopWidget::OnBuyResult(EShopBuyResult Result, FName ItemId)
{
    if (DetailWidget)
    {
        DetailWidget->RefreshStock();
    }
}
