// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUDs/PG_LobbyWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"
#include "Components/CanvasPanelSlot.h"
#include "PlayerState/LobbyPlayerState.h"
#include "Components/EditableTextBox.h"

void UPG_LobbyWidget::SetInit()
{
}

void UPG_LobbyWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    CheckPlayerState();

    if (GameStartButton)
    {
        GameStartButton->OnHovered.AddDynamic(
            this, &ThisClass::HandleGameStartHovered);

        GameStartButton->OnUnhovered.AddDynamic(
            this, &ThisClass::HandleGameStartUnhovered);

        GameStartButton->OnClicked.AddDynamic(
            this, &ThisClass::HandleGameStartClicked);
    }

    if (MatchingInfoButton)
    {
        MatchingInfoButton->OnHovered().AddUObject(
            this, &ThisClass::HandleMatchingInfoHovered);

        MatchingInfoButton->OnUnhovered().AddUObject(
            this, &ThisClass::HandleMatchingInfoUnhovered);

        MatchingInfoButton->OnClicked().AddUObject(
            this, &ThisClass::HandleMatchingInfoCancelRequested);
    }

    if (InputNickName)
    {
        InputNickName->OnTextChanged.AddDynamic(this, &ThisClass::HandleNickNameTextChanged);
    }

    if (SelectNickName)
    {
        SelectNickName->OnClicked.AddDynamic(this, &ThisClass::HandleSelectNickNameClicked);
    }
        
    if (GameStartButtonText)
    {
        GameStartButtonText->SetText(FText::FromString(TEXT("GameStart")));
    }

    ReadyState = ETitleReadyState::Closed;
    bIsMoving = false;

    PlayAnimation(StartGame, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
    ApplyPosition(MatchingInfoClosedPos);
}

void UPG_LobbyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bIsMoving || !MatchingInfoButton)
        return;

    MoveElapsed += InDeltaTime;
    float Alpha = FMath::Clamp(MoveElapsed / MoveDuration, 0.f, 1.f);

    FVector2D NewPos = FMath::Lerp(MoveStartPos, MoveTargetPos, Alpha);
    ApplyPosition(NewPos);

    if (Alpha >= 1.f)
    {
        bIsMoving = false;
        ApplyPosition(MoveTargetPos);
    }
}

void UPG_LobbyWidget::HandleGameStartClicked()
{
    switch (ReadyState)
    {
    case ETitleReadyState::Closed:
        //if (ModeSelectButton)
        //    ModeSelectButton->SetIsEnabled(false);
        if (GameStartButtonText)
            GameStartButtonText->SetText(FText::FromString(TEXT("Cancel")));
        SetReadyState(ETitleReadyState::InfoOnly);
        break;
    case ETitleReadyState::InfoOnly:
    case ETitleReadyState::Armed:
        //if (ModeSelectButton)
        //    ModeSelectButton->SetIsEnabled(true);
        if (GameStartButtonText)
            GameStartButtonText->SetText(FText::FromString(TEXT("GameStart")));
        SetReadyState(ETitleReadyState::Closed);
        break;
    default:
        break;
    }

}

void UPG_LobbyWidget::HandleGameStartHovered()
{
    if (ReadyState == ETitleReadyState::Closed)
        return;

    if (GameStartButtonText)
        GameStartButtonText->SetText(FText::FromString(TEXT("Cancel")));
}

void UPG_LobbyWidget::HandleGameStartUnhovered()
{
    if (ReadyState == ETitleReadyState::Closed)
        return;

    if (GameStartButtonText)
        GameStartButtonText->SetText(FText::FromString(TEXT("Maching")));
}

void UPG_LobbyWidget::HandleMatchingInfoHovered()
{
    if (ReadyState == ETitleReadyState::Closed)
        return;

    SetReadyState(ETitleReadyState::Armed);
}

void UPG_LobbyWidget::HandleMatchingInfoUnhovered()
{
    if (ReadyState == ETitleReadyState::Closed)
        return;

    SetReadyState(ETitleReadyState::InfoOnly);
}

void UPG_LobbyWidget::HandleMatchingInfoCancelRequested()
{
    SetReadyState(ETitleReadyState::Closed);

    //if (ModeSelectButton)
    //    ModeSelectButton->SetIsEnabled(true);

    if (GameStartButtonText)
        GameStartButtonText->SetText(FText::FromString(TEXT("GameStart")));
}

void UPG_LobbyWidget::HandleNickNameTextChanged(const FText& NewText)
{
    UpdateSelectNickNameEnabled();
}

void UPG_LobbyWidget::HandleSelectNickNameClicked()
{
    PlayAnimation(SetNickName, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);

    if (!CheckPlayerState())
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerState is not ready yet!"));
        return;
    }

    if (!InputNickName)
    {
        return;
    }

    LobbyPlayerState->ServerSetNickName(InputNickName->GetText().ToString());
}

void UPG_LobbyWidget::UpdateSelectNickNameEnabled()
{
    if (!SelectNickName || !InputNickName)
        return;

    FString Str = InputNickName->GetText().ToString();
    Str.TrimStartAndEndInline();

    const bool bEnable = !Str.IsEmpty();
    SelectNickName->SetIsEnabled(bEnable);
}

bool UPG_LobbyWidget::CheckPlayerState()
{
    if (LobbyPlayerState)
        return true;

    ALobbyPlayerState* FoundPS = Cast<ALobbyPlayerState>(GetOwningPlayerState());
    if (!FoundPS)
        return false;

    LobbyPlayerState = FoundPS;
    return true;
}

void UPG_LobbyWidget::StartMoveTo(const FVector2D& TargetPos)
{
    if (!MatchingInfoButton)
        return;

    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MatchingInfoButton->Slot))
    {
        MoveStartPos = CanvasSlot->GetPosition();
        MoveTargetPos = TargetPos;
        MoveElapsed = 0.f;
        bIsMoving = true;
    }
}

void UPG_LobbyWidget::ApplyPosition(const FVector2D& NewPos)
{
    if (!MatchingInfoButton)
        return;

    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MatchingInfoButton->Slot))
    {
        CanvasSlot->SetPosition(NewPos);
    }
}

void UPG_LobbyWidget::SetReadyState(ETitleReadyState NewState)
{
    if (ReadyState == NewState)
        return;
    ETitleReadyState originState = ReadyState;
    ReadyState = NewState;

    switch (ReadyState)
    {
    case ETitleReadyState::Closed:
        StartMoveTo(MatchingInfoClosedPos);

        if (CheckPlayerState())
            LobbyPlayerState.Get()->ServerSetLobbyState(EPlayerLobbyState::PLS_NotReady);
        else
            UE_LOG(LogTemp, Warning, TEXT("PlayerState is not ready yet!"));
        break;

    case ETitleReadyState::InfoOnly:
        StartMoveTo(MatchingInfoInfoOnlyPos);

        if (originState == ETitleReadyState::Armed)
            return;

        if(CheckPlayerState())
            LobbyPlayerState->ServerSetLobbyState(EPlayerLobbyState::PLS_MatchingReady);
        else
            UE_LOG(LogTemp, Warning, TEXT("PlayerState is not ready yet!"));
        break;

    case ETitleReadyState::Armed:
        StartMoveTo(MatchingInfoArmedPos);
        break;
    }
}
