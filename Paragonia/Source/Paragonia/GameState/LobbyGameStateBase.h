// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LobbyGameStateBase.generated.h"

UENUM(BlueprintType)
enum class EGameLobbyState : uint8
{
    GLS_WaitingForPlayers,
    GLS_CharacterSelect,
    GLS_GameStarting,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyGameStateChangedDelegate, EGameLobbyState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLeftTimeChangedDelegate, int32, NewTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchWaitCountChangedDelegate, int32, NewCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchWaitCountMaxChangedDelegate, int32, NewCount);

/**
 * 
 */
UCLASS()
class PARAGONIA_API ALobbyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
public:
	ALobbyGameStateBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(BlueprintCallable, Category = "LobbyState")
	EGameLobbyState GetCurrentLobbyState() const { return CurrentLobbyState; }

	UFUNCTION(BlueprintCallable, Category = "LobbyState")
	int32 GetLeftTime() const { return LeftTime; }

	UFUNCTION(BlueprintCallable, Category = "LobbyState")
	int32 GetMatchingWaitUserCount() const { return MatchingWaitUserCount; }

	UFUNCTION(BlueprintCallable, Category = "LobbyState")
	int32 GetMatchingWaitUserCountMax() const { return MatchingWaitUserCountMax; }

	// Only Server Set
	void SetLobbyState(EGameLobbyState NewState);
	void SetLeftTime(int32 NewTime);
	void SetMatchingWaitUserCount(int32 NewCount);
	void SetMatchingWaitUserCountMax(int32 NewMatchingWaitUserCountMax);
public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLobbyGameStateChangedDelegate OnLobbyGameStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLeftTimeChangedDelegate OnLeftTimeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMatchWaitCountChangedDelegate OnMatchWaitCountChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMatchWaitCountChangedDelegate OnMatchWaitCountMaxChanged;

protected:
	UFUNCTION()
	void OnRep_CurrentLobbyState();

	UFUNCTION()
	void OnRep_LeftTime();

	UFUNCTION()
	void OnRep_MatchWaitCount();

	UFUNCTION()
	void OnRep_MatchWaitCountMax();
protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentLobbyState)
	EGameLobbyState CurrentLobbyState;

	UPROPERTY(ReplicatedUsing = OnRep_LeftTime)
	int32 LeftTime;

	UPROPERTY(ReplicatedUsing = OnRep_MatchWaitCount)
	int32 MatchingWaitUserCount;

	UPROPERTY(ReplicatedUsing = OnRep_MatchWaitCountMax)
	int32 MatchingWaitUserCountMax;
};
