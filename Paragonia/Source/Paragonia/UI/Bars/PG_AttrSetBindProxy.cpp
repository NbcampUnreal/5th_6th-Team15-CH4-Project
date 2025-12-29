// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Bars/PG_AttrSetBindProxy.h"
#include "AttributeSet/CharacterAttributeSet.h"
#include "UI/HUDs/PG_IngameHUD.h"

void UPG_AttrSetBindProxy::Init(UPG_IngameHUD* InHUD, EHPBarSlot InSlot, UCharacterAttributeSet* InSet)
{
    HUD = InHUD;
    Slot = InSlot;

    if (BoundSet == InSet)
    {
        return;
    }

    Unbind();

    BoundSet = InSet;
    if (!IsValid(BoundSet) || !IsValid(HUD))
    {
        return;
    }

    BoundSet->OnHealthChanged_UI.AddDynamic(this, &ThisClass::OnHealth);
    BoundSet->OnMaxHealthChanged_UI.AddDynamic(this, &ThisClass::OnMaxHealth);

    OnMaxHealth(BoundSet->GetMaxHealth(), BoundSet->GetMaxHealth());
    OnHealth(BoundSet->GetHealth(), BoundSet->GetHealth());
}

void UPG_AttrSetBindProxy::Unbind()
{
    if (!IsValid(BoundSet))
    {
        return;
    }

    BoundSet->OnHealthChanged_UI.RemoveDynamic(this, &ThisClass::OnHealth);
    BoundSet->OnMaxHealthChanged_UI.RemoveDynamic(this, &ThisClass::OnMaxHealth);
    BoundSet = nullptr;
}


void UPG_AttrSetBindProxy::OnHealth(float OldValue, float NewValue)
{
    if (IsValid(HUD))
    {
        HUD->HandleHealthChangedBySlot(Slot, OldValue, NewValue);
    }
}

void UPG_AttrSetBindProxy::OnMaxHealth(float OldValue, float NewValue)
{
    if (IsValid(HUD))
    {
        HUD->HandleMaxHealthChangedBySlot(Slot, OldValue, NewValue);
    }
}