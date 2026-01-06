// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Panels/PG_IngameInfo.h"
#include "AttributeSet/CharacterAttributeSet.h"
#include "UI/Bars/PG_HPBar.h"
#include "Components/TextBlock.h"
#include "UI/Inventory/PGInventorySlotWidget.h"
#include "Inventory/PGInventoryComponent.h"

void UPG_IngameInfo::SetHPBarColor(bool TeamType)
{
	if (PlayerHPBar)
	{
		PlayerHPBar->SetTeamColor(TeamType);
	}
}

void UPG_IngameInfo::InitInventory(UPGInventoryComponent* InInventoryComponent)
{
	if (InventoryComponent != InInventoryComponent)
	{
		UnbindInventory();
	}

	if (!IsValid(InInventoryComponent))
	{
		return;
	}

	InventoryComponent = InInventoryComponent;

	UPGInventorySlotWidget* Items[] = { Item0, Item1, Item2, Item3, Item4, Item5 };

	int index = 0;

	for (UPGInventorySlotWidget* Item : Items)
	{
		if (!IsValid(Item))
		{
			continue;
		}

		Item->Init(InInventoryComponent, index++);
		Item->SetVisibility(ESlateVisibility::Hidden);
	}

	InventoryComponent->OnInventoryChanged.RemoveAll(this);
	InventoryComponent->OnInventoryChanged.AddUObject(this, &UPG_IngameInfo::RefreshAll);

	RefreshAll();
}

void UPG_IngameInfo::RefreshAll()
{
	UPGInventorySlotWidget* Items[] = { Item0, Item1, Item2, Item3, Item4, Item5 };

	for (UPGInventorySlotWidget* Item : Items)
	{
		if (!IsValid(Item))
		{
			continue;
		}

		const bool bHasItem = Item->Refresh();
		Item->SetVisibility(bHasItem ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

		if (bHasItem)
		{
			UE_LOG(LogTemp, Warning, TEXT("Init"));
		}
	}
}

void UPG_IngameInfo::UnbindInventory()
{
	if (!InventoryComponent)
	{
		return;
	}

	InventoryComponent->OnInventoryChanged.RemoveAll(this);
	InventoryComponent = nullptr;

}

void UPG_IngameInfo::SetNickName(const FString& InNickName, bool CheckTeam)
{
	FString Nickname = InNickName;
	Nickname.TrimStartAndEndInline();

	if (Nickname.IsEmpty())
	{
		Nickname = TEXT("Unknown");
	}

	if (PlayerName)
	{
		PlayerName->SetText(FText::FromString(Nickname));
		if (CheckTeam == false)
		{
			PlayerName->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
		}
	}
}

void UPG_IngameInfo::BindToAttributeSet(UCharacterAttributeSet* InAttrSet)
{
	if (BoundAttrSet == InAttrSet)
	{
		return;
	}

	UnbindFromAttributeSet();
	
	BoundAttrSet = InAttrSet;
	if (!IsValid(BoundAttrSet))
	{
		return;
	}

	BoundAttrSet->OnHealthChanged_UI.AddDynamic(this, &ThisClass::HandleHealthChanged);
	BoundAttrSet->OnMaxHealthChanged_UI.AddDynamic(this, &ThisClass::HandleMaxHealthChanged);

	HandleMaxHealthChanged(BoundAttrSet->GetMaxHealth(), BoundAttrSet->GetMaxHealth());
	HandleHealthChanged(BoundAttrSet->GetHealth(), BoundAttrSet->GetHealth());
}

void UPG_IngameInfo::HandleHealthChanged(float OldValue, float NewValue)
{
	if (PlayerHPBar)
	{
		PlayerHPBar->HandleHealthChanged(OldValue, NewValue);
	}
}


void UPG_IngameInfo::HandleMaxHealthChanged(float OldValue, float NewValue)
{
	if (PlayerHPBar)
	{
		PlayerHPBar->HandleMaxHealthChanged(OldValue, NewValue);
	}
}

void UPG_IngameInfo::NativeDestruct()
{
	UnbindFromAttributeSet();
	UnbindInventory();
	Super::NativeDestruct();
}

void UPG_IngameInfo::UnbindFromAttributeSet()
{
	if (!IsValid(BoundAttrSet))
	{
		return;
	}

	BoundAttrSet->OnHealthChanged_UI.RemoveDynamic(this, &ThisClass::HandleHealthChanged);
	BoundAttrSet->OnMaxHealthChanged_UI.RemoveDynamic(this, &ThisClass::HandleMaxHealthChanged);

	BoundAttrSet = nullptr;
}
