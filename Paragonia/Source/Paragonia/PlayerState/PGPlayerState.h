#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PGPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, int32 /*NewGold*/);

class UPGInventoryComponent;
class UPGInventoryWidget;

UCLASS()
class PARAGONIA_API APGPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	APGPlayerState();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	int32 GetCharID() const { return CharacterID; }
	int32 GetTeamID() const { return TeamID; }
	int32 GetPlayerNumberId() const { return PlayerNumberId; }
	const FString& GetPlayerNickName() const { return PlayerNickName; }

	void SetCharID(int32 NewCharID);
	void SetTeamID(int32 NewTeamID);
	void SetPlayerNickName(const FString& NewPlayerNickName);
	void SetPlayerNumberId(int32 NewPlayerNumberId);
	int32 GetGold() const { return Gold; }

	UPGInventoryComponent* GetInventoryComponent() { return Inventory; }

protected:
	UPROPERTY(VisibleAnywhere, Replicated)
	int32 CharacterID;
	UPROPERTY(VisibleAnywhere, Replicated)
	int32 TeamID;
	UPROPERTY(VisibleAnywhere, Replicated)
	FString PlayerNickName;
	UPROPERTY(Replicated)
	int32 PlayerNumberId;

public:
	UPROPERTY(VisibleAnywhere, Replicated)
	bool bClientReady = false;


#pragma region Shop
public:
	FOnGoldChanged OnGoldChanged;

	UFUNCTION()
	void OnRep_Gold();

	bool CanAfford(int32 Cost) const { return Gold >= Cost; }
	void AddGold(int32 Delta);

protected:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Gold, BlueprintReadOnly)
	int32 Gold = 400;
#pragma endregion Shop

#pragma region Inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UPGInventoryComponent> Inventory;

#pragma endregion Inventory
};