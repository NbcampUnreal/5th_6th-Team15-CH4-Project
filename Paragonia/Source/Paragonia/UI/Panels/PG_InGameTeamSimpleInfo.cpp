// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Panels/PG_InGameTeamSimpleInfo.h"
#include "UI/Icons/PG_PlayerIcon.h"
#include "UI/Bars/PG_HPBar.h"


void UPG_InGameTeamSimpleInfo::InitTeamSimpleInfo(int32 CharacterID)
{
	SetVisibility(ESlateVisibility::HitTestInvisible);
	if (PlayerIcon)
	{
		PlayerIcon->SetPlayerIcon(CharacterID);
		PlayerIcon->ApplyIcon();
	}
}

void UPG_InGameTeamSimpleInfo::InitTeamSimpleInfo(int32 CharacterID, const FString& PlayerName)
{
	SetVisibility(ESlateVisibility::HitTestInvisible);
	if (PlayerIcon)
	{
		PlayerIcon->SetPlayerIcon(CharacterID);
		PlayerIcon->SetPlayerNameText(PlayerName);
		PlayerIcon->ApplyIcon();
	}
}

void UPG_InGameTeamSimpleInfo::HandleHealthChanged(float OldValue, float NewValue)
{
	if (HPBar)
	{
		HPBar->HandleHealthChanged(OldValue, NewValue);
	}
}

void UPG_InGameTeamSimpleInfo::HandleMaxHealthChanged(float OldValue, float NewValue)
{
	if (HPBar)
	{
		HPBar->HandleMaxHealthChanged(OldValue, NewValue);
	}
}

void UPG_InGameTeamSimpleInfo::AddBuff(int32 buffUID)
{
	if (HPBar)
	{
		HPBar->AddBuff(buffUID);
	}
}

void UPG_InGameTeamSimpleInfo::SetTeamColor()
{
	if (HPBar)
	{
		HPBar->SetTeamColor(0);
	}
}
