#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PGGameStateBase.generated.h"

UENUM(BlueprintType)
enum class ETeamResult : uint8 { None, Team1Win, Team2Win};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamResultChanged, ETeamResult, NewResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameTimeUpdated, int32, NewTime);

UCLASS()
class PARAGONIA_API APGGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	APGGameStateBase();

#pragma region Team
public:
    UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_TeamResult)
    ETeamResult TeamResult = ETeamResult::None;

    UPROPERTY(BlueprintAssignable)
    FOnTeamResultChanged OnTeamResultChanged;

    UFUNCTION()
    void OnRep_TeamResult();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma endregion Team


#pragma region GameTime
public:

	UPROPERTY(BlueprintAssignable, Category = "GameTime")
	FOnGameTimeUpdated OnGameTimeUpdated;

	void StartGameTimer();

	UFUNCTION(BlueprintPure, Category = "GameTime")
	int32 GetCurrentGameTime() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentGameTime, Category = "GameTime")
	int32 CurrentGameTime;

	FTimerHandle GameTimerHandle;

	void UpdateTimer();

	UFUNCTION()
	void OnRep_CurrentGameTime();

public:
#pragma endregion GameTime

};
