// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUDs/PG_IngameHUD.h"
#include "UI/Panels/PG_InGameTeamSimpleInfo.h"
#include "UI/Bars/PG_HPBar.h"
#include "UI/Icons/PG_SkillIcon.h"
#include "AttributeSet/CharacterAttributeSet.h"
#include "Character/PGPlayerCharacterBase.h"
#include "GameplayTagContainer.h"
#include "UI/MiniMap/PG_MiniMap.h"
#include "AttributeSet/CharacterAttributeSet.h"

void UPG_IngameHUD::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    HPBars.Add(EHPBarSlot::Player, PlayerHPBar);
    HPBars.Add(EHPBarSlot::Team1,  Team1HPBar->GetTeamHPBar());
    HPBars.Add(EHPBarSlot::Team2, Team2HPBar->GetTeamHPBar());
    HPBars.Add(EHPBarSlot::OurNexus, OurNexusHPBar);
    HPBars.Add(EHPBarSlot::EnemyNexus, EnemyNexusHPBar);

	BindCooldownToSkillIcon();
}

void UPG_IngameHUD::NativeDestruct()
{
    for (auto& Pair : BindProxies)
    {
        if (IsValid(Pair.Value))
        {
            Pair.Value->Unbind();
        }
    }
    BindProxies.Empty();
    BoundAttrSets.Empty();
    HPBars.Empty();

    Super::NativeDestruct();
}

void UPG_IngameHUD::BindSlot(EHPBarSlot InSlot, UCharacterAttributeSet* Set)
{
    BoundAttrSets.FindOrAdd(InSlot) = Set;

    UPG_AttrSetBindProxy* Proxy = BindProxies.FindRef(InSlot);
    if (!IsValid(Proxy))
    {
        Proxy = NewObject<UPG_AttrSetBindProxy>(this);
        BindProxies.Add(InSlot, Proxy);
    }

    Proxy->Init(this, InSlot, Set);

    if (UPG_HPBar* Bar = HPBars.FindRef(InSlot))
    {
        switch (InSlot)
        {
        case EHPBarSlot::Player: Bar->SetPlayerColor(); break;
        case EHPBarSlot::Team1:  Bar->SetTeamColor(0); break;
        case EHPBarSlot::Team2:  Bar->SetTeamColor(0); break;
        case EHPBarSlot::OurNexus:  Bar->SetTeamColor(0); break;
        case EHPBarSlot::EnemyNexus:  Bar->SetTeamColor(1); break;
        default: break;
        }
    }
}

void UPG_IngameHUD::HandleHealthChangedBySlot(EHPBarSlot InSlot, float OldValue, float NewValue)
{
    if (UPG_HPBar* Bar = HPBars.FindRef(InSlot))
    {
        float Value = Bar->HandleHealthChanged(OldValue, NewValue);

        if (InSlot == EHPBarSlot::Player && Value < 0.3f)
        {
            PlayAnimation(OnDamaged, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
        }

    }
}

void UPG_IngameHUD::HandleMaxHealthChangedBySlot(EHPBarSlot InSlot, float OldValue, float NewValue)
{
    if (UPG_HPBar* Bar = HPBars.FindRef(InSlot))
    {
        Bar->HandleMaxHealthChanged(OldValue, NewValue);
    }
}

void UPG_IngameHUD::InitMinimap(UTextureRenderTarget2D* InRT)
{
    if (!MiniMapWidget || !InRT)
    {
        return;
    }

    MiniMapWidget->SetMinimapRenderTarget(InRT);
}

void UPG_IngameHUD::InitTeam1IngameIcon(int32 CharacterID)
{
    Team1HPBar->InitTeamSimpleInfo(CharacterID);
}

void UPG_IngameHUD::InitTeam2IngameIcon(int32 CharacterID)
{
    Team2HPBar->InitTeamSimpleInfo(CharacterID);
}

void UPG_IngameHUD::HandleCooldownTimeChanged(FGameplayTag CooldownTag, float Remaining, float Duration)
{
    if (TObjectPtr<UPG_SkillIcon>* Found = CooldownTagToWidget.Find(CooldownTag))
    {
        if (UPG_SkillIcon* SkillIcon = Found->Get())
        {
            if (Remaining > 0.f && Duration > 0.f)
            {
                SkillIcon->UpdateCooldown(Remaining, Duration);
            }
            else
            {
                SkillIcon->ClearCooldown();
            }
        }
    }
}

void UPG_IngameHUD::HandleCooldownTagChanged(FGameplayTag CooldownTag, int32 NewCount)
{
    if (TObjectPtr<UPG_SkillIcon>* Found = CooldownTagToWidget.Find(CooldownTag))
    {
        if (UPG_SkillIcon* SkillIcon = Found->Get())
        {
            if (NewCount > 0)
            {
                SkillIcon->StartCooldown();
            }
            else
            {
                SkillIcon->ClearCooldown();
            }
        }
	}
}

void UPG_IngameHUD::BindCooldownToSkillIcon()
{
    APGPlayerCharacterBase* PlayerCharacter = Cast<APGPlayerCharacterBase>(GetOwningPlayerPawn());
    if (!IsValid(PlayerCharacter))
    {
        return;
    }

    const FGameplayTag QCooldownTag = FGameplayTag::RequestGameplayTag(FName("Cooldown.Skill.Q"));
    const FGameplayTag ECooldownTag = FGameplayTag::RequestGameplayTag(FName("Cooldown.Skill.E"));
    const FGameplayTag RCooldownTag = FGameplayTag::RequestGameplayTag(FName("Cooldown.Skill.R"));

    CooldownTagToWidget.Add(QCooldownTag, SkillIconQ);
    CooldownTagToWidget.Add(ECooldownTag, SkillIconE);
    CooldownTagToWidget.Add(RCooldownTag, SkillIconR);

    if (SkillIconQ)
    {
        SkillIconQ->InitSlot(QCooldownTag);
    }

    if (SkillIconE)
    {
        SkillIconE->InitSlot(ECooldownTag);
	}

    if (SkillIconR)
    {
        SkillIconR->InitSlot(RCooldownTag);
	}

	PlayerCharacter->OnCooldownTimeChangedDelegate.AddDynamic(this, &ThisClass::HandleCooldownTimeChanged);
	PlayerCharacter->OnCooldownTagChangedDelegate.AddDynamic(this, &ThisClass::HandleCooldownTagChanged);
}
