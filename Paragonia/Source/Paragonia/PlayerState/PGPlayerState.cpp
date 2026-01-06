#include "PlayerState/PGPlayerState.h"
#include "AttributeSet/CharacterAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "Character/PGPlayerCharacterBase.h"
#include "Inventory/PGInventoryComponent.h"

APGPlayerState::APGPlayerState()
{
	bReplicates = true;

	Inventory = CreateDefaultSubobject<UPGInventoryComponent>(TEXT("Inventory"));

}

void APGPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void APGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGPlayerState, TeamID);
	DOREPLIFETIME(APGPlayerState, CharacterID);
	DOREPLIFETIME(APGPlayerState, bClientReady);
	DOREPLIFETIME(APGPlayerState, Gold);
	DOREPLIFETIME(APGPlayerState, PlayerNickName);
	DOREPLIFETIME(APGPlayerState, PlayerNumberId);
}

void APGPlayerState::SetCharID(int32 NewCharID)
{
	this->CharacterID = NewCharID;
}

void APGPlayerState::SetTeamID(int32 NewTeamID)
{
	this->TeamID = NewTeamID;
}

void APGPlayerState::SetPlayerNickName(const FString& NewPlayerNickName)
{
	this->PlayerNickName = NewPlayerNickName;
}

void APGPlayerState::SetPlayerNumberId(int32 NewPlayerNumberId)
{
	this->PlayerNumberId = NewPlayerNumberId;
}
void APGPlayerState::OnRep_Gold()
{
	OnGoldChanged.Broadcast(Gold);
}

void APGPlayerState::AddGold(int32 Delta)
{
	Gold = FMath::Max(0, Gold + Delta);
	OnRep_Gold();
}