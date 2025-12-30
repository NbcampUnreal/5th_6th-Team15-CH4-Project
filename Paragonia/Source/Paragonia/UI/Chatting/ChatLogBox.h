#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "ChatLogBox.generated.h"

class UTextBlock;

UCLASS()
class PARAGONIA_API UChatLogBox : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerNameTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LogTextBlock;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chatting")
	FColor AllyColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chatting")
	FColor EnemyColor;

public:
	void SetInfo(const FString& PlayerName, const FString& Log, const int32& InTeamID);
	void SetPlayerName(const FString& Name, const int32& InTeamID);
	void SetLog(const FString& Log);
	
};
