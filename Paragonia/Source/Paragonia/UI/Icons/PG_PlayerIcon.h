// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PG_PlayerIcon.generated.h"

class UImage;
class UTextBlock;
class UBorder;
class UPGCharacterDescriptionSubsystem;
/**
 * 
 */
UCLASS()
class PARAGONIA_API UPG_PlayerIcon : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	int32 ApplyIcon();

	void SetPlayerIcon(const int32 CharacterUID);
	void SetPlayerIcon(const FName& CharacterName);

	UFUNCTION(BlueprintCallable, Category = "PlayerIcon")
	void SetPlayerNameText(const FText& InName);

	UFUNCTION(BlueprintCallable, Category = "LobbyData")
	int32 GetPlayerNumberId() const { return PlayerNumberID; }

	UFUNCTION(BlueprintCallable, Category = "LobbyData")
	void SetPlayerNumberId(int32 InPlayerNumberID);

protected:
	virtual void NativeOnInitialized() override;
	void ApplyIconTexture(UTexture2D* Tex);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> PlayerNameBase;

	UPROPERTY(Transient)
	TObjectPtr<UPGCharacterDescriptionSubsystem> CharacterDescSubsys;

	int32 PlayerNumberID = -1;
	int32 SelectCharacterID = -1;
};
