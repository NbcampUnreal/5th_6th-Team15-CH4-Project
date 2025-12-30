#include "UI/Shop/PGShopItemEntryWidget.h"
#include "Components/Image.h"
#include "Shop/PGShopItemObject.h" 
#include "Components/Border.h"

void UPGShopItemEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    const UPGShopItemObject* Obj = Cast<UPGShopItemObject>(ListItemObject);
    if (!Obj) 
    {
        return;
    }

    const FPGShopItemRow& RowData = Obj->Data;

    if (IconImage)
    {
        if (RowData.Icon.IsNull())
        {
            IconImage->SetBrushFromTexture(nullptr);
        }
        else
        {
            UTexture2D* Tex = RowData.Icon.LoadSynchronous();
            IconImage->SetBrushFromTexture(Tex);
        }
    }

    if (SelectionBorder)
    {
        SelectionBorder->SetVisibility(ESlateVisibility::Hidden);
    }

    if (InLineBorder)
    {
        FLinearColor NewColor;
        switch (RowData.Rarity)
        {
            case EItemRarity::Common:    NewColor = FLinearColor(0.6f, 0.6f, 0.6f, 0.5f); break;
            case EItemRarity::Uncommon:  NewColor = FLinearColor(0.2f, 1.0f, 0.2f, 0.5f); break;
            case EItemRarity::Rare:      NewColor = FLinearColor(0.2f, 0.5f, 1.0f, 0.5f); break;
            case EItemRarity::Unique:    NewColor = FLinearColor(0.7f, 0.2f, 1.0f, 0.5f); break;
            case EItemRarity::Legendary: NewColor = FLinearColor(1.0f, 0.6f, 0.0f, 0.5f); break;
            default:                     NewColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f); break;
        }

        InLineBorder->SetBrushColor(NewColor);
    }
}

void UPGShopItemEntryWidget::NativeOnItemSelectionChanged(bool bIsSelected)
{
    if (SelectionBorder)
    {
        if (bIsSelected)
        {
            SelectionBorder->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            SelectionBorder->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

FLinearColor UPGShopItemEntryWidget::GetColorByRarity(EItemRarity Rarity)
{
    switch (Rarity)
    {
        case EItemRarity::Common:    return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f); // 회색
        case EItemRarity::Uncommon:  return FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // 초록
        case EItemRarity::Rare:      return FLinearColor(0.0f, 0.5f, 1.0f, 1.0f); // 파랑
        case EItemRarity::Unique:    return FLinearColor(0.6f, 0.0f, 1.0f, 1.0f); // 보라
        case EItemRarity::Legendary: return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // 주황/골드
        default:                     return FLinearColor::White;
    }
}