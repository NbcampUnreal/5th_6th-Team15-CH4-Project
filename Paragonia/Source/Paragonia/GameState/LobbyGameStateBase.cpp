// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/LobbyGameStateBase.h"
#include "Net/UnrealNetwork.h"

ALobbyGameStateBase::ALobbyGameStateBase()
	:LeftTime(0),
	MatchingWaitUserCount(0),
	MatchingWaitUserCountMax(0)
{
	CurrentLobbyState = EGameLobbyState::GLS_WaitingForPlayers;
}

void ALobbyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentLobbyState);
	DOREPLIFETIME(ThisClass, LeftTime);
	DOREPLIFETIME(ThisClass, MatchingWaitUserCount);
	DOREPLIFETIME(ThisClass, MatchingWaitUserCountMax);
}

void ALobbyGameStateBase::SetLobbyState(EGameLobbyState NewState)
{
	if (HasAuthority())
	{
		CurrentLobbyState = NewState;
		OnRep_CurrentLobbyState();
	}
}

void ALobbyGameStateBase::SetLeftTime(int32 NewTime)
{
	if (HasAuthority())
	{
		LeftTime = NewTime;
		OnRep_LeftTime();
	}
}

void ALobbyGameStateBase::SetMatchingWaitUserCount(int32 NewCount)
{
	if (HasAuthority())
	{
		MatchingWaitUserCount = NewCount;
		OnRep_MatchWaitCount();
	}
}

void ALobbyGameStateBase::SetMatchingWaitUserCountMax(int32 NewMatchingWaitUserCountMax)
{
	if (HasAuthority())
	{
		MatchingWaitUserCountMax = NewMatchingWaitUserCountMax;
	}
}

void ALobbyGameStateBase::OnRep_CurrentLobbyState()
{
	if (OnLobbyGameStateChanged.IsBound())
	{
		OnLobbyGameStateChanged.Broadcast(CurrentLobbyState);
	}
}

void ALobbyGameStateBase::OnRep_LeftTime()
{
	if (OnLeftTimeChanged.IsBound())
	{
		OnLeftTimeChanged.Broadcast(LeftTime);
	}
}

void ALobbyGameStateBase::OnRep_MatchWaitCount()
{
	if (OnMatchWaitCountChanged.IsBound())
	{
		OnMatchWaitCountChanged.Broadcast(MatchingWaitUserCount);
	}
}

void ALobbyGameStateBase::OnRep_MatchWaitCountMax()
{
	if (OnMatchWaitCountMaxChanged.IsBound())
	{
		OnMatchWaitCountMaxChanged.Broadcast(MatchingWaitUserCountMax);
	}
}
