#include "GameState/PGGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/PGPlayerState.h"
#include "Controller/PGPlayerController.h"

APGGameStateBase::APGGameStateBase()
	:CurrentGameTime(0)
{
	bReplicates = true;
}
	
void APGGameStateBase::OnRep_TeamResult()
{
	OnTeamResultChanged.Broadcast(TeamResult);
}	

void APGGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGGameStateBase, TeamResult);
	DOREPLIFETIME(APGGameStateBase, CurrentGameTime);
}

#pragma region GameTime

void APGGameStateBase::StartGameTimer()
{
	if (HasAuthority())
	{
		CurrentGameTime = 0;
		OnRep_CurrentGameTime();
		GetWorldTimerManager().SetTimer(GameTimerHandle, this, &APGGameStateBase::UpdateTimer, 1.0f, true);
	}
}

int32 APGGameStateBase::GetCurrentGameTime() const
{
	return CurrentGameTime;
}

void APGGameStateBase::UpdateTimer()
{
	CurrentGameTime++;
	OnRep_CurrentGameTime();
}

void APGGameStateBase::OnRep_CurrentGameTime()
{
	if (OnGameTimeUpdated.IsBound())
	{
		OnGameTimeUpdated.Broadcast(CurrentGameTime);
	}
}

#pragma endregion GameTime