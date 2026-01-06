// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Shop/PGShopTypes.h"
#include "PGInventorySlotWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotClicked, int32 /*SlotIndex*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotRightClicked, int32 /*SlotIndex*/);

class UImage;
class UTextBlock;
class UPGInventoryComponent;
class UBorder;

UCLASS()
class PARAGONIA_API UPGInventorySlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void Init(UPGInventoryComponent* InInventory, int32 InSlotIndex);

    bool Refresh();

    void SetSelected(bool bIsSelected);

    UPROPERTY(EditDefaultsOnly, Category = "DragDrop")
    TSubclassOf<UUserWidget> DragVisualWidgetClass;

    FOnSlotClicked OnSlotClicked;
    FOnSlotClicked OnSlotRightClicked;

protected:
    virtual void NativeConstruct() override;

    // Drag/Drop
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

    void SetEmptyVisual();
   
    FLinearColor GetColorByRarity(EItemRarity Rarity);

protected:
    UPROPERTY(meta = (BindWidget)) UImage* IconImage;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* CountText;
    
    UPROPERTY(meta = (BindWidgetOptional))
    UBorder* InLineBorder;

    UPROPERTY(meta = (BindWidget))
    UBorder* SelectionBorder;

    UPROPERTY() TObjectPtr<UPGInventoryComponent> Inventory;
    int32 SlotIndex = INDEX_NONE;

   
};