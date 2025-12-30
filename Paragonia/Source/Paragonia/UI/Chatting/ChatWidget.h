#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "ChatWidget.generated.h"

class UVerticalBox;
class UScrollBox;
class UChatLogBox;
class UEditableTextBox;

UCLASS()
class PARAGONIA_API UChatWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chatting")
	TSubclassOf<UChatLogBox> LogBoxClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> LogList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> LogListScroll;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEditableTextBox> ChatInput;

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnChatInputTextcommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ReturnToGame();

	virtual UChatLogBox* CreateLogBox();
	virtual void AddLog(const FString& PlayerName, const FString& Log, int32 InTeamID);
};
