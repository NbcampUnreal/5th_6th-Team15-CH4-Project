// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TitlePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PARAGONIA_API ATitlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	void ConnectLobby();

	UFUNCTION()
	void OnLoginSuccess();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UI, Meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> UIWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = UI, Meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> UIWidgetInstance;
};
