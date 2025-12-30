#include "UI/Chatting/ChatWidget.h"

#include "UI/Chatting/ChatLogBox.h"
#include "Controller/PGPlayerController.h"

#include "Components/EditableTextBox.h"
#include "Components/VerticalBox.h"
#include "Components/ScrollBox.h"
#include "Blueprint/WidgetTree.h"

void UChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextcommitted))
	{
		ChatInput->OnTextCommitted.AddDynamic(this, &ThisClass::OnChatInputTextcommitted);

		ChatInput->SetText(FText());
	}
}

void UChatWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (!ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextcommitted))
	{
		ChatInput->OnTextCommitted.RemoveDynamic(this, &ThisClass::OnChatInputTextcommitted);
	}
}

void UChatWidget::OnChatInputTextcommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		APlayerController* PlayerController = GetOwningPlayer();
		if (!IsValid(PlayerController))
		{
			return;
		}

		APGPlayerController* PGPC = Cast<APGPlayerController>(PlayerController);
		if (!IsValid(PGPC))
		{
			return;
		}

		PGPC->SetChatMessageString(Text.ToString());

		ReturnToGame();
	}
}

void UChatWidget::ReturnToGame_Implementation()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!IsValid(PlayerController))
	{
		return;
	}

	APGPlayerController* PGPC = Cast<APGPlayerController>(PlayerController);
	if (!IsValid(PGPC))
	{
		return;
	}

	PGPC->SetInputMode(FInputModeGameOnly());
	PGPC->SetShowMouseCursor(false);
}

UChatLogBox* UChatWidget::CreateLogBox()
{
	if (!IsValid(LogBoxClass)) return nullptr;

	return WidgetTree->ConstructWidget<UChatLogBox>(LogBoxClass);
}

void UChatWidget::AddLog(const FString& PlayerName, const FString& Log, int32 InTeamID)
{
	UChatLogBox* LogBox = CreateLogBox();
	if (!IsValid(LogBox)) return;

	LogBox->SetInfo(PlayerName, Log, InTeamID);

	LogList->AddChildToVerticalBox(LogBox);
	FMargin Margin;
	Margin.Left = 10.0f;
	Margin.Right = 10.0f;
	Margin.Top = 10.0f;
	Margin.Bottom = 10.0f;
	LogBox->SetPadding(Margin);

	LogListScroll->ScrollToEnd();
}
