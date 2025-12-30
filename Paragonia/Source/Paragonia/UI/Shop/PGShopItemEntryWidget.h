#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Shop/PGShopTypes.h"
#include "PGShopItemEntryWidget.generated.h"

class UImage;
class UBorder;

UCLASS()
class PARAGONIA_API UPGShopItemEntryWidget : public UUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

    virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;

protected:
    FLinearColor GetColorByRarity(EItemRarity Rarity);

    UPROPERTY(meta = (BindWidget)) 
    UImage* IconImage;

    UPROPERTY(meta = (BindWidget))
    UBorder* SelectionBorder;

    UPROPERTY(meta = (BindWidgetOptional))
    UBorder* InLineBorder;
};
