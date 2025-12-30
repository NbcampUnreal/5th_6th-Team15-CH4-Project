// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "ConnectSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoginSuccessDelegate);

/**
 * 
 */
UCLASS()
class PARAGONIA_API UConnectSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void Login();

	void CreateGameSession();

	UFUNCTION(BlueprintCallable, Category = "Network|Client")
	void FindAndJoinSession();

	UFUNCTION(BlueprintCallable, Category = "Network|Server")
	void TravelToGame();

	UFUNCTION(BlueprintCallable, Category = "Network|Server")
	void TravelToLobby();

	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnLoginSuccessDelegate OnLoginSuccessDelegate;

	bool IsPlayerLoggedIn() const { return bIsLoggedIn; }

private:
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	bool bIsLoggedIn = false;
};
