// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/TitlePlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Subsystem/ConnectSubsystem.h"
#include "Engine/GameInstance.h"

void ATitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

	if (IsValid(UIWidgetClass) == true)
	{
		UIWidgetInstance = CreateWidget<UUserWidget>(this, UIWidgetClass);
		if (IsValid(UIWidgetInstance) == true)
		{
			UIWidgetInstance->AddToViewport();

			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(UIWidgetInstance->GetCachedWidget());
			SetInputMode(Mode);

			bShowMouseCursor = true;
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (IsValid(GameInstance))
	{
		UConnectSubsystem* ConnectSubsystem = GameInstance->GetSubsystem<UConnectSubsystem>();
		if (IsValid(ConnectSubsystem))
		{
			if (ConnectSubsystem->IsPlayerLoggedIn())
			{
				UE_LOG(LogTemp, Log, TEXT("[TitlePC] Already Logged In. Connecting..."));
				ConnectLobby();
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("[TitlePC] Waiting for Login..."));
				ConnectSubsystem->OnLoginSuccessDelegate.AddDynamic(this, &ATitlePlayerController::OnLoginSuccess);
			}
		}
	}
}

void ATitlePlayerController::ConnectLobby()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (IsValid(GameInstance) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TitlePC] Game Instance is Not Valid...?"));
		return;
	}

	UConnectSubsystem* ConnectSubsystem = GameInstance->GetSubsystem<UConnectSubsystem>();
	if (IsValid(ConnectSubsystem))
	{
		UE_LOG(LogTemp, Log, TEXT("[TitlePC] Requesting connection to Lobby..."));

		ConnectSubsystem->FindAndJoinSession();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[TitlePC] ConnectSubsystem is invalid!"));
	}
}

void ATitlePlayerController::OnLoginSuccess()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UConnectSubsystem* Sub = GameInstance->GetSubsystem<UConnectSubsystem>();
		if (Sub) Sub->OnLoginSuccessDelegate.RemoveDynamic(this, &ATitlePlayerController::OnLoginSuccess);
	}

	ConnectLobby();
}
