// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PG_LobbyWidget.generated.h"

class UCommonButtonBase;
class UButton;
class UTextBlock;
class ALobbyPlayerState;
class UEditableTextBox;

UENUM(BlueprintType)
enum class ETitleReadyState : uint8
{
    Closed,
    InfoOnly,
    Armed,
};

/**
 * 
 */
UCLASS()
class PARAGONIA_API UPG_LobbyWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
    void SetInit();

protected:
    UPROPERTY()
    TObjectPtr<ALobbyPlayerState> LobbyPlayerState;

    UPROPERTY(BlueprintReadOnly, Category = "Matching")
    ETitleReadyState ReadyState = ETitleReadyState::Closed;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> MatchingInfoButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> GameStartButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> GameStartButtonText; 

    //UPROPERTY(meta = (BindWidget))
    //TObjectPtr<UButton> ModeSelectButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> SelectNickName;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEditableTextBox> InputNickName;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> StartGame;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> SetNickName;

    UPROPERTY(EditAnywhere, Category = "Matching")
    FVector2D MatchingInfoClosedPos;

    UPROPERTY(EditAnywhere, Category = "Matching")
    FVector2D MatchingInfoInfoOnlyPos;

    UPROPERTY(EditAnywhere, Category = "Matching")
    FVector2D MatchingInfoArmedPos;

    virtual void NativeOnInitialized() override;

    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION()
    void HandleGameStartClicked();

    UFUNCTION()
    void HandleGameStartHovered();

    UFUNCTION()
    void HandleGameStartUnhovered();

    UFUNCTION()
    void HandleMatchingInfoHovered();

    UFUNCTION()
    void HandleMatchingInfoUnhovered();

    UFUNCTION()
    void HandleMatchingInfoCancelRequested();

    UFUNCTION()
    void HandleNickNameTextChanged(const FText& NewText);

    UFUNCTION()
    void HandleSelectNickNameClicked();

    bool CheckPlayerState();

    void StartMoveTo(const FVector2D& TargetPos);
    void ApplyPosition(const FVector2D& NewPos);
    void SetReadyState(ETitleReadyState NewState);
    void UpdateSelectNickNameEnabled();
private:

    bool bIsMoving = false;
    FVector2D MoveStartPos;
    FVector2D MoveTargetPos;
    float MoveElapsed = 0.f;
    float MoveDuration = 0.25f;
};
