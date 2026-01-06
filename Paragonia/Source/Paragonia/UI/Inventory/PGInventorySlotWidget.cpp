#include "UI/Inventory/PGInventorySlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

#include "UI/Inventory/PGInventoryDragOp.h"
#include "Inventory/PGInventoryComponent.h"  
#include "UI/Inventory/PGInventoryDragImage.h"

#include "Shop/PGShopTypes.h" 
#include "Engine/DataTable.h"

void UPGInventorySlotWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (SelectionBorder)
    {
        SelectionBorder->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UPGInventorySlotWidget::Init(UPGInventoryComponent* InInventory, int32 InSlotIndex)
{
    Inventory = InInventory;
    SlotIndex = InSlotIndex;
    Refresh();
}

void UPGInventorySlotWidget::SetSelected(bool bIsSelected)
{
    if (SelectionBorder)
    {
        if (bIsSelected)
        {
            // 보더가 보이되, 클릭을 가로채지 않도록 HitTestInvisible 권장
            SelectionBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            SelectionBorder->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

bool UPGInventorySlotWidget::Refresh()
{
    bool Ret = false;

    if (!Inventory || SlotIndex == INDEX_NONE)
    {
        SetEmptyVisual();
        return Ret;
    }

    if (!Inventory->Slots.IsValidIndex(SlotIndex))
    {
        SetEmptyVisual();
        return Ret;
    }

    const FInventorySlot& InvSlot = Inventory->Slots[SlotIndex];

    if (IconImage)
    {
        if (!InvSlot.bEmpty && !InvSlot.ItemId.IsNone())
        {
            UDataTable* DT = Inventory->GetItemDataTable();
            if (DT)
            {
                static const FString Context(TEXT("InventorySlotIcon"));
                const FPGShopItemRow* Row = DT->FindRow<FPGShopItemRow>(InvSlot.ItemId, Context);

                UTexture2D* IconTex = Row ? Row->Icon.LoadSynchronous() : nullptr;

                if (IconTex)
                {
                    IconImage->SetBrushFromTexture(IconTex);
                    IconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
                    Ret = true;
                }
                else
                {
                    IconImage->SetBrushFromTexture(nullptr);
                    IconImage->SetVisibility(ESlateVisibility::Hidden);
                }

                if (Row)
                {
                    if (InLineBorder)
                    {
                        InLineBorder->SetBrushColor(GetColorByRarity(Row->Rarity));
                        // 혹은 SetColorAndOpacity 사용
                    }
                }
                else
                {
                    // 아이템 없으면 기본색(투명 혹은 어두운색)
                    if (InLineBorder)
                    {
                        InLineBorder->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
                    }
                }
            }
        }
        else
        {
            IconImage->SetBrushFromTexture(nullptr);
            IconImage->SetVisibility(ESlateVisibility::Hidden);
        }
    }
    if (CountText)
    {
        CountText->SetText(FText::AsNumber(InvSlot.Count));
        CountText->SetVisibility(InvSlot.Count > 1 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }

    return Ret;
}

void UPGInventorySlotWidget::SetEmptyVisual()
{
    if (IconImage)
    {
        IconImage->SetBrushFromTexture(nullptr);
    }
    if (CountText)
    {
        CountText->SetText(FText::GetEmpty());
        CountText->SetVisibility(ESlateVisibility::Hidden);
    }
}

FReply UPGInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    const FKey Button = InMouseEvent.GetEffectingButton();

    if (Button == EKeys::RightMouseButton)
    {
        OnSlotRightClicked.Broadcast(SlotIndex);
        return FReply::Handled();
    }

    if (Button == EKeys::LeftMouseButton)

    {
        OnSlotClicked.Broadcast(SlotIndex);

        return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UPGInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    if (!Inventory || !Inventory->Slots.IsValidIndex(SlotIndex))
    {
        return;
    }

    const FInventorySlot& InvSlot = Inventory->Slots[SlotIndex];
    if (InvSlot.bEmpty)
    {
        return;
    }

    UPGInventoryDragOp* DragOp = NewObject<UPGInventoryDragOp>();
    DragOp->FromIndex = SlotIndex;

    if (DragVisualWidgetClass)
    {
        UPGInventoryDragImage* DragVisual = CreateWidget<UPGInventoryDragImage>(GetWorld(), DragVisualWidgetClass);

        UTexture2D* SlotTexture = nullptr;
        
        UDataTable* DT = Inventory->GetItemDataTable();
        if (DT)
        {
            static const FString Context(TEXT("InventorySlotIcon"));
            const FPGShopItemRow* Row = DT->FindRow<FPGShopItemRow>(InvSlot.ItemId, Context);
            SlotTexture = Row ? Row->Icon.LoadSynchronous() : nullptr;
        }

        if (DragVisual && SlotTexture)
        {
            DragVisual->SetItemImage(SlotTexture);
        }

        DragOp->DefaultDragVisual = DragVisual;
    }

    DragOp->Pivot = EDragPivot::MouseDown;

    OutOperation = DragOp;
}

bool UPGInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    UPGInventoryDragOp* DragOp = Cast<UPGInventoryDragOp>(InOperation);
    if (!DragOp || !Inventory)
    {
        return false;
    }

    if (DragOp)
    {
        const int32 From = DragOp->FromIndex;
        const int32 To = SlotIndex;

        if (From == INDEX_NONE || To == INDEX_NONE)
        {
            return false;
        }
        if (From == To)
        {
            return true;
        }

        Inventory->RequestSwapSlot(From, To);
        return true;
    }
    return false;
}

FLinearColor UPGInventorySlotWidget::GetColorByRarity(EItemRarity Rarity)
{
    switch (Rarity)
    {
        case EItemRarity::Common:    return FLinearColor(0.6f, 0.6f, 0.6f, 0.5f); break;
        case EItemRarity::Uncommon:  return FLinearColor(0.2f, 1.0f, 0.2f, 0.5f); break;
        case EItemRarity::Rare:      return FLinearColor(0.2f, 0.5f, 1.0f, 0.5f); break;
        case EItemRarity::Unique:    return FLinearColor(0.7f, 0.2f, 1.0f, 0.5f); break;
        case EItemRarity::Legendary: return FLinearColor(1.0f, 0.6f, 0.0f, 0.5f); break;
        default:                     return FLinearColor(0.2f, 0.2f, 0.2f, 1.0f); break;
    }
}