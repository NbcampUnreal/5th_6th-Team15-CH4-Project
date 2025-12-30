// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameState/LobbyGameStateBase.h"
#include "LobbyPlayerState.generated.h"

UENUM(BlueprintType)
enum class EPlayerLobbyState : uint8
{
	PLS_NotReady,
	PLS_MatchingReady,
	PLS_Selecting,
	PLS_SelectedAndReady,
	PLS_GameStartWait,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyPlayerStateChangedDelegate, EPlayerLobbyState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterIDChangedDelegate, int32, NewCharacterID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamIDChangedDelegate, int32, NewTeamID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchWaitTimeChangedDelegate, int32, NewTime);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLobbyUIHelperDelegate, EPlayerLobbyState, NewState, int32, PlayerUID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNickNameChangedDelegate, const FString&, NewNickName);

/**
 *
 */
UCLASS()
class PARAGONIA_API ALobbyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void CopyProperties(APlayerState* PlayerState) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	UFUNCTION(BlueprintCallable, Category = "LobbyData")
	EPlayerLobbyState GetPlayerLobbyState() const { return PlayerLobbyState; }

	UFUNCTION(BlueprintCallable, Category = "LobbyData")
	int32 GetCharacterID() const { return CharacterID; }

	UFUNCTION(BlueprintCallable, Category = "LobbyData")
	int32 GetTeamID() const { return TeamID; }

	UFUNCTION(BlueprintCallable, Category = "LobbyData")
	int32 GetMatchWaitTime() const { return MatchWaitTime; }

	UFUNCTION(BlueprintCallable, Category = "LobbyData")
	int32 GetPlayerNumberId() const { return PlayerNumberId; }

	UFUNCTION(BlueprintCallable, Category = "LobbyData")
	const FString& GetPlayerNickName() const { return PlayerNickName; }

	// Client -> Server RPC
	UFUNCTION(Server, Reliable)
	void ServerSetLobbyState(EPlayerLobbyState NewState);

	UFUNCTION(Server, Reliable)
	void ServerSetCharacterID(int32 NewID);

	UFUNCTION(Server, Reliable)
	void ServerSetTeamID(int32 NewTeamID);

	UFUNCTION(Server, Reliable)
	void ServerSetNickName(const FString& NewNickName);

	// PlayerID는 접속시에 한 번 정해지기에 Setter
	void SetPlayerNumberId(int32 NewID);

	UFUNCTION()
	void OnGSLobbyStateChangedHandler(EGameLobbyState NewState);

public:
	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLobbyPlayerStateChangedDelegate OnLobbyPlayerStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCharacterIDChangedDelegate OnCharacterIDChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTeamIDChangedDelegate OnTeamIDChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMatchWaitTimeChangedDelegate OnMatchTimeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNickNameChangedDelegate OnNickNameChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLobbyUIHelperDelegate OnLobbyPlayerStateChangedHelperUI;

protected:
	UFUNCTION()
	void OnRep_PlayerLobbyState();

	UFUNCTION()
	void OnRep_CharacterID();

	UFUNCTION()
	void OnRep_TeamID();

	UFUNCTION()
	void OnRep_MatchWaitTime();

	UFUNCTION()
	void OnRep_PlayerNickName();

protected:
	UFUNCTION(Category = "Lobby|Matching")
	void StartMatchingTimer();

	UFUNCTION(Category = "Lobby|Matching")
	void StopMatchingTimer();

	void IncreaseWaitTime();

	void CheckStateForTimer();
protected:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerLobbyState)
	EPlayerLobbyState PlayerLobbyState;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CharacterID)
	int32 CharacterID;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_TeamID)
	int32 TeamID;

	UPROPERTY(ReplicatedUsing = OnRep_MatchWaitTime)
	int32 MatchWaitTime;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerNickName)
	FString PlayerNickName;

	UPROPERTY(Replicated)
	int32 PlayerNumberId;

	FTimerHandle TimerHandle_MatchWait;
};
