#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PGGameModeBase.generated.h"

class APGPlayerCharacterBase;
class APGPlayerController;
class AController;
class APGPlayerState;
class APGNexus;

USTRUCT(BlueprintType)
struct FPGRespawnInfo
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<APGPlayerController> Controller;

    UPROPERTY()
    float RespawnTime = 0.f;
};

UCLASS()
class PARAGONIA_API APGGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    APGGameModeBase();

    virtual void PostLogin(APlayerController* NewPlayer) override;

    virtual void Logout(AController* Exiting) override;

    virtual void BeginPlay() override;

    virtual void HandleSeamlessTravelPlayer(AController*& C) override;

    virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

    virtual void PostSeamlessTravel() override;


    FTransform GetTeamSpawnTransform(int32 TeamID) const;

    void CheckAllClientsReady();
protected:
    UPROPERTY(EditDefaultsOnly, Category = "Character")
    TMap<int32, TSubclassOf<APawn>> CharacterIDToPawnClass;

    UPROPERTY(EditDefaultsOnly, Category = "PG|Loading")
    int32 RequiredPlayers = 3;

    UPROPERTY(EditDefaultsOnly, Category = "PG|Loading")
    float ReadyTimeoutSeconds = 30.f;


    void StartReadyPolling();
    void StopReadyPolling();
    void TickReadyPolling();
    void TryStartReadyCountdown();
    void OnReadyTimeout();

    FTimerHandle ReadyTimeoutTimerHandle;
    FTimerHandle ReadyPollTimerHandle;
    bool bReadyCountdownStarted = false;
    bool bAllClientsReady = false;
    bool bAbortInProgress = false;

#pragma region DeathAndRespawn
public:
    /** 캐릭터 사망 시 호출 (Character에서 HandleDeath에서 호출) */
    void HandleCharacterDeath(APGPlayerCharacterBase* DeadCharacter, AController* InstigatorController);

protected:
    void TickRespawn();

    void RespawnPlayer(APGPlayerController* Controller);

    ///** 주기적으로 Respawn 체크 */
    FTimerHandle RespawnTimerHandle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<FPGRespawnInfo> PendingRespawns;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<TObjectPtr<APGPlayerController>> AlivePlayerControllers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<TObjectPtr<APGPlayerController>> DeadPlayerControllers;

    ///** 부활 관련 설정 */
    UPROPERTY(EditDefaultsOnly, Category = "PG|Respawn")
    float BaseRespawnTime = 5.f;

    //UPROPERTY(EditDefaultsOnly, Category = "PG|Respawn")
    //float RespawnTimePerLevel = 1.f;
#pragma endregion DeathAndRespawn

#pragma region Nexus
public: 
    // 사실상 GameOver
    UFUNCTION()
    void OnObjectiveDestroyed(AActor* DestroyedActor);

protected:
    UPROPERTY()
    bool bIsGameWin;


#pragma endregion

#pragma region ReturnLobby
protected:
    FTimerHandle EndGameTimerHandle;

    void ReturnToLobby();

    UPROPERTY(EditDefaultsOnly, Category = "PG|Lobby")
    float ReturnLobbyTime = 5.f;
#pragma endregion

#pragma region Chatting

public:
    void PrintChatMessageString(APGPlayerController* InChattingPlayerController, const FString& InChatMessageString, const int32& InTeamID);


#pragma endregion


};