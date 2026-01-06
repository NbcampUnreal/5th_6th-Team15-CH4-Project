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
#include "UI/Chatting/ChatWidget.h"
#include "UI/Inventory/PGInventorySlotWidget.h"
#include "Inventory/PGInventoryComponent.h"
#include "UI/Panels/PG_KillLog.h"
#include "PlayerState/PGPlayerState.h"
#include "Components/TextBlock.h"

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

void UPG_IngameHUD::NativeConstruct()
{
    Super::NativeConstruct();
    InitKillLogSlots();
}

void UPG_IngameHUD::InitKillLogSlots()
{
    KillLogSlots = { KillLog0, KillLog1, KillLog2, KillLog3, KillLog4, KillLog5 };
    KillLogMap.Empty();

    for (UPG_KillLog* KillLog : KillLogSlots)
    {
        if (!IsValid(KillLog))
        {
            continue;
        }
        KillLog->ResetSlot();
        KillLog->SetVisibility(ESlateVisibility::Hidden);
    }
}

UPG_KillLog* UPG_IngameHUD::GetOrAssignSlot(int32 PlayerId)
{
    if (PlayerId == INDEX_NONE)
    {
        return nullptr;
    }

    if (TObjectPtr<UPG_KillLog>* Found = KillLogMap.Find(PlayerId))
    {
        return Found->Get();
    }

    for (UPG_KillLog* KillLog : KillLogSlots)
    {
        if (!IsValid(KillLog))
        {
            continue;
        }

        const bool bUsed = KillLogMap.FindKey(KillLog) != nullptr;
        if (bUsed)
        {
            continue;
        }

        KillLogMap.Add(PlayerId, KillLog);
        return KillLog;
    }

    return nullptr;
}


void UPG_IngameHUD::OnKillEvent(APGPlayerState* ClientPS, APGPlayerState* KillerPS, APGPlayerState* VictimPS)
{
    if (!IsValid(ClientPS))
    {
        return;
    }

    if (IsValid(VictimPS))
    {
        const int32 VictimId = VictimPS->GetPlayerId();
        if (UPG_KillLog* Log = GetOrAssignSlot(VictimId))
        {
            Log->InitIfNeeded(ClientPS, VictimPS);
            Log->ShowKillLog(KillerPS);
        }
    }
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
    UnbindInventory();
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

void UPG_IngameHUD::InitTeam1IngameIcon(int32 CharacterID, const FString& PlayerNickName1)
{
    Team1HPBar->InitTeamSimpleInfo(CharacterID, PlayerNickName1);
}

void UPG_IngameHUD::InitTeam2IngameIcon(int32 CharacterID, const FString& PlayerNickName2)
{
    Team2HPBar->InitTeamSimpleInfo(CharacterID, PlayerNickName2);
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

void UPG_IngameHUD::InitInventory(UPGInventoryComponent* InInventoryComponent)
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
    InventoryComponent->OnInventoryChanged.AddUObject(this, &UPG_IngameHUD::RefreshAll);

    RefreshAll();

}

void UPG_IngameHUD::RefreshAll()
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

void UPG_IngameHUD::UnbindInventory()
{
    if (IsValid(InventoryComponent))
    {
        InventoryComponent->OnInventoryChanged.RemoveAll(this);
    }

    InventoryComponent = nullptr;
}

void UPG_IngameHUD::HandleGoldChange(int32 NewGold)
{
    if (GoldText)
    {
        GoldText->SetText(FText::FromString(FString::Printf(TEXT("Gold: %d"), NewGold)));
    }
}

void UPG_IngameHUD::InitGold(APGPlayerState* InPS)
{
    if (!IsValid(InPS))
    {
        return;
    }

    InPS->OnGoldChanged.RemoveAll(this);

    InPS->OnGoldChanged.AddUObject(this, &UPG_IngameHUD::HandleGoldChange);

    HandleGoldChange(InPS->GetGold());
}

#pragma region Chatting

void UPG_IngameHUD::PrintChatMessageString(const FString& PlayerName, const FString& InChatMessageString, const int32& InTeamID)
{
    if (!IsValid(ChatWidget)) return;

    ChatWidget->AddLog(PlayerName, InChatMessageString, InTeamID);
}

#pragma endregion

