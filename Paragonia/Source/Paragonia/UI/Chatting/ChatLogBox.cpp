#include "UI/Chatting/ChatLogBox.h"

#include "Character/PGPlayerCharacterBase.h"

#include "Components/TextBlock.h"

void UChatLogBox::SetInfo(const FString& PlayerName, const FString& Log, const int32& InTeamID)
{
	SetPlayerName(PlayerName, InTeamID);
	SetLog(Log);
}

void UChatLogBox::SetPlayerName(const FString& PlayerName, const int32& InTeamID)
{
	if (!IsValid(PlayerNameTextBlock)) return;

	PlayerNameTextBlock->SetText(FText::FromString(PlayerName));

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (IsValid(LocalPlayer))
	{
		UWorld* World = GetWorld();
		if (IsValid(World))
		{
			APlayerController* PC = LocalPlayer->GetPlayerController(World);
			if (IsValid(PC))
			{
				APawn* Pawn = PC->GetPawn();
				if (IsValid(Pawn))
				{
					APGPlayerCharacterBase* Player = Cast<APGPlayerCharacterBase>(Pawn);
					if (IsValid(Player))
					{
						int32 PlayerTeamID = IPGTeamStatusInterface::Execute_GetTeamID(Player);
						if (PlayerTeamID == InTeamID)
						{
							PlayerNameTextBlock->SetColorAndOpacity(AllyColor);
						}
						else
						{
							PlayerNameTextBlock->SetColorAndOpacity(EnemyColor);
						}
					}
				}
			}
		}
	}
}

void UChatLogBox::SetLog(const FString& Log)
{
	if (!IsValid(LogTextBlock)) return;

	LogTextBlock->SetText(FText::FromString(Log));
}


