// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/LobbyPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/PGPlayerState.h"
#include "GameMode/LobbyGameModeBase.h"

void ALobbyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	MatchWaitTime = 0;

	if (auto* GS = GetWorld()->GetGameState<ALobbyGameStateBase>())
	{
		GS->OnLobbyGameStateChanged.AddDynamic(this, &ALobbyPlayerState::OnGSLobbyStateChangedHandler);
	}
}

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PlayerLobbyState);
	DOREPLIFETIME(ThisClass, CharacterID);
	DOREPLIFETIME(ThisClass, TeamID);
	DOREPLIFETIME(ThisClass, MatchWaitTime);
	DOREPLIFETIME(ThisClass, PlayerNumberId);
	DOREPLIFETIME(ThisClass, PlayerNickName);
}

void ALobbyPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	APGPlayerState* TargetPS = Cast<APGPlayerState>(PlayerState);
	if (TargetPS)
	{
		UE_LOG(LogTemp, Log, TEXT("[CopyProperties] Name : %s, CharacterID : %d, TeamID : %d!"), *GetPlayerName(), CharacterID, TeamID);
		TargetPS->SetCharID(CharacterID);
		TargetPS->SetTeamID(TeamID);
		TargetPS->SetPlayerNumberId(PlayerNumberId);
		TargetPS->SetPlayerNickName(PlayerNickName);
	}
}

void ALobbyPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (auto* GS = GetWorld()->GetGameState<ALobbyGameStateBase>())
	{
		GS->OnLobbyGameStateChanged.RemoveDynamic(this, &ALobbyPlayerState::OnGSLobbyStateChangedHandler);
	}

	Super::EndPlay(EndPlayReason);
}

void ALobbyPlayerState::ServerSetLobbyState_Implementation(EPlayerLobbyState NewState)
{
	if (NewState == PlayerLobbyState)
		return;

	PlayerLobbyState = NewState;

	OnRep_PlayerLobbyState();

	CheckStateForTimer();
}

void ALobbyPlayerState::ServerSetCharacterID_Implementation(int32 NewID)
{
	if (NewID < 0)
		return;

	if (NewID == CharacterID)
		return;

	CharacterID = NewID;

	OnRep_CharacterID();
}

void ALobbyPlayerState::ServerSetTeamID_Implementation(int32 NewTeamID)
{
	if (NewTeamID < 0)
		return;

	if (NewTeamID == TeamID)
		return;

	TeamID = NewTeamID;

	OnRep_TeamID();
}


void ALobbyPlayerState::ServerSetNickName_Implementation(const FString& NewNickName)
{
	FString Str = NewNickName;
	Str.TrimStartAndEndInline();

	if (Str.IsEmpty())
		return;

	Str = Str.Left(12);

	if (PlayerNickName == Str)
		return;

	PlayerNickName = Str;

	OnRep_PlayerNickName();
}

void ALobbyPlayerState::SetPlayerNumberId(int32 NewID)
{
	PlayerNumberId = NewID;
}

void ALobbyPlayerState::OnGSLobbyStateChangedHandler(EGameLobbyState NewState)
{
	switch (NewState)
	{
	case EGameLobbyState::GLS_WaitingForPlayers:
		break;
	case EGameLobbyState::GLS_CharacterSelect:
		ServerSetLobbyState(EPlayerLobbyState::PLS_Selecting);
		break;
	case EGameLobbyState::GLS_GameStarting:
		ServerSetLobbyState(EPlayerLobbyState::PLS_GameStartWait);
		break;
	default:
		break;
	}
}

void ALobbyPlayerState::OnRep_PlayerLobbyState()
{
	if (OnLobbyPlayerStateChanged.IsBound())
	{
		OnLobbyPlayerStateChanged.Broadcast(PlayerLobbyState);
	}

	if (OnLobbyPlayerStateChangedHelperUI.IsBound())
	{
		OnLobbyPlayerStateChangedHelperUI.Broadcast(PlayerLobbyState, PlayerNumberId);
	}
}

void ALobbyPlayerState::OnRep_CharacterID()
{
	if (OnCharacterIDChanged.IsBound())
	{
		OnCharacterIDChanged.Broadcast(CharacterID);
	}
}

void ALobbyPlayerState::OnRep_TeamID()
{
	if (OnTeamIDChanged.IsBound())
	{
		OnTeamIDChanged.Broadcast(TeamID);
	}
}

void ALobbyPlayerState::OnRep_MatchWaitTime()
{
	if (OnMatchTimeChanged.IsBound())
	{
		OnMatchTimeChanged.Broadcast(MatchWaitTime);
	}
}

void ALobbyPlayerState::OnRep_PlayerNickName()
{
	if (OnNickNameChanged.IsBound())
	{
		OnNickNameChanged.Broadcast(PlayerNickName);
	}
}

void ALobbyPlayerState::StartMatchingTimer()
{
	if (HasAuthority())
	{
		MatchWaitTime = 0;
		GetWorldTimerManager().ClearTimer(TimerHandle_MatchWait);
		GetWorldTimerManager().SetTimer(TimerHandle_MatchWait, this, &ALobbyPlayerState::IncreaseWaitTime, 1.0f, true);
		OnRep_MatchWaitTime();
	}
}

void ALobbyPlayerState::StopMatchingTimer()
{
	if (HasAuthority())
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_MatchWait);
		MatchWaitTime = 0;
		OnRep_MatchWaitTime();
	}
}

void ALobbyPlayerState::IncreaseWaitTime()
{
	MatchWaitTime++;
}

void ALobbyPlayerState::CheckStateForTimer()
{
	if (PlayerLobbyState == EPlayerLobbyState::PLS_MatchingReady)
	{
		StartMatchingTimer();
	}
	else
	{
		StopMatchingTimer();
	}
}