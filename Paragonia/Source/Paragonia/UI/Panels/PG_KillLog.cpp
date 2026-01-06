// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Panels/PG_KillLog.h"
#include "Components/TextBlock.h"
#include "UI/Icons/PG_PlayerIcon.h"
#include "PlayerState/PGPlayerState.h"
#include "Components/Image.h"

void UPG_KillLog::ResetSlot()
{
    bInited = false;
    BoundPlayerId = INDEX_NONE;

    if (KillerIcon)
    {
        KillerIcon->SetVisibility(ESlateVisibility::Hidden);
    }
    if (LogLabel)
    {
        LogLabel->SetText(FText::GetEmpty());
    }

    StopAnimation(SetInit);
    PlayAnimation(SetInit, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
}
void UPG_KillLog::InitIfNeeded(APGPlayerState* InPlayerState, APGPlayerState* DeathPlayerState)
{
    if (!IsValid(InPlayerState) || !IsValid(DeathPlayerState))
    {
        return;
    }

    const int32 PlayerId = DeathPlayerState->GetPlayerId();
    if (bInited && BoundPlayerId == PlayerId)
    {
        return;
    }

    if (InPlayerState->GetTeamID() != DeathPlayerState->GetTeamID())
    {
        if (IsValid(TeamImage))
        {
            TeamImage->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (VictimIcon)
    {
        VictimIcon->SetPlayerIcon(DeathPlayerState->GetCharID());
        KillerIcon->ApplyIcon();
    }

    BoundPlayerId = PlayerId;
    bInited = true;
}

void UPG_KillLog::ShowKillLog(APGPlayerState* KillerPlayerState)
{
    if (KillerIcon)
    {
        if (KillerPlayerState == nullptr)
        {
            KillerIcon->SetVisibility(ESlateVisibility::Hidden);
        }
        else
        {
            KillerIcon->SetVisibility(ESlateVisibility::Visible);
            KillerIcon->SetPlayerIcon(KillerPlayerState->GetCharID());
            KillerIcon->ApplyIcon();
        }
    }

    if (PulseAnim)
    {
        this->SetVisibility(ESlateVisibility::Visible);
        StopAnimation(PulseAnim);
        PlayAnimation(PulseAnim, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
    }

}
