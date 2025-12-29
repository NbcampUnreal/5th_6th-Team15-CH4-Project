// PGGameMode.cpp

#include "PGGameModeBase.h"
#include "Character/PGPlayerCharacterBase.h"
#include "PlayerState/PGPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameState/PGGameStateBase.h"
#include "Controller/PGPlayerController.h"
#include "EngineUtils.h"
#include "Object/PGNexus.h"
#include "Character/PGPlayerCharacterBase.h"
#include "PlayerStart/PGPlayerStart.h"
#include "Subsystem/ConnectSubsystem.h"

APGGameModeBase::APGGameModeBase()
{
    bUseSeamlessTravel = true;
    // 기본 Pawn, PlayerController 클래스 설정은 여기서
}

// 로그인 시 플레이어 컨트롤러를 생존자 목록에 추가
void APGGameModeBase::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    APGGameStateBase* PGGameState = GetGameState<APGGameStateBase>();
    if (IsValid(PGGameState) == false)
    {
        return;
    }

    APGPlayerController* NewPlayerController = Cast<APGPlayerController>(NewPlayer);
    if (IsValid(NewPlayerController) == true)
    {
        AlivePlayerControllers.AddUnique(NewPlayerController);
        NewPlayerController->Client_SetExpectedPlayerCount(RequiredPlayers);
    }
}

// 로그아웃 시 플레이어 컨트롤러를 생존자 목록에서 제거하고 사망자 목록에 추가
void APGGameModeBase::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    APGPlayerController* ExitingPlayerController = Cast<APGPlayerController>(Exiting);
    if (IsValid(ExitingPlayerController) == true && AlivePlayerControllers.Find(ExitingPlayerController) != INDEX_NONE)
    {
        AlivePlayerControllers.Remove(ExitingPlayerController);
        DeadPlayerControllers.Add(ExitingPlayerController);
    }
}

// 게임 시작 시 부활 체크 타이머 설정
void APGGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    // 0.5초마다 부활 체크
    GetWorldTimerManager().SetTimer(
        RespawnTimerHandle,
        this,
        &APGGameModeBase::TickRespawn,
        0.5f,
        true
    );

    for (TActorIterator<APGNexus> It(GetWorld()); It; ++It)
    {
        APGNexus* Nexus = *It;
        if (Nexus && Nexus->ActorHasTag(FName("Nexus")))
        {
            Nexus->OnNexusDestroyed.AddDynamic(this, &APGGameModeBase::OnObjectiveDestroyed);
        }
    }
}

void APGGameModeBase::HandleSeamlessTravelPlayer(AController*& C)
{
    Super::HandleSeamlessTravelPlayer(C);

   
    APGPlayerController* NewPlayerController = Cast<APGPlayerController>(C);
    if (IsValid(NewPlayerController) == true)
    {
        AlivePlayerControllers.AddUnique(NewPlayerController);        
        NewPlayerController->Client_SetExpectedPlayerCount(RequiredPlayers);
    }
}

UClass* APGGameModeBase::GetDefaultPawnClassForController_Implementation(AController* InController)
{
    if (APGPlayerState* PS = InController ? InController->GetPlayerState<APGPlayerState>() : nullptr)
    {
        if (TSubclassOf<APawn>* Found = CharacterIDToPawnClass.Find(PS->GetCharID()))
        {
            return Found->Get();
        }
    }

    return Super::GetDefaultPawnClassForController_Implementation(InController);
}

AActor* APGGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
    int32 PlayerTeamID = 0;
    if (Player && Player->PlayerState)
    {
        PlayerTeamID = Cast<APGPlayerState>(Player->PlayerState)->GetTeamID();
    }

    for (TActorIterator<APGPlayerStart> It(GetWorld()); It; ++It)
    {
        if (It->TeamID == PlayerTeamID)
        {
            UE_LOG(LogTemp, Warning, TEXT("APGGameModeBase::ChoosePlayerStart: Return PlayerStart"));
            return *It;
        }
    }

    return Super::ChoosePlayerStart_Implementation(Player);
}

void APGGameModeBase::PostSeamlessTravel()
{
    Super::PostSeamlessTravel();

    bAllClientsReady = false;
    bAbortInProgress = false;
    bReadyCountdownStarted = false;
    GetWorldTimerManager().ClearTimer(ReadyTimeoutTimerHandle);

    AlivePlayerControllers.Reset();
    DeadPlayerControllers.Reset();

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APGPlayerController* PC = Cast<APGPlayerController>(It->Get());
        if (!IsValid(PC))
        {
            continue;
        }

        AlivePlayerControllers.AddUnique(PC);

        PC->Client_SetExpectedPlayerCount(RequiredPlayers);

        if (APGPlayerState* PS = PC->GetPlayerState<APGPlayerState>())
        {
            PS->bClientReady = false;
        }
    }

    TryStartReadyCountdown();
    StartReadyPolling();
}

FTransform APGGameModeBase::GetTeamSpawnTransform(int32 TeamID) const
{
    for (TActorIterator<APGPlayerStart> It(GetWorld()); It; ++It)
    {
        if (It->TeamID == TeamID)
        {
            return It->GetActorTransform();
        }
    }
    return FTransform(FRotator::ZeroRotator, FVector::ZeroVector);
}

void APGGameModeBase::CheckAllClientsReady()
{
    if (bAllClientsReady || bAbortInProgress)
    {
        return;
    }
    if (GetNumPlayers() < RequiredPlayers)
    {
        return;
    }

    AGameStateBase* GS = GameState;
    if (!IsValid(GS))
    {
        return;
    }

    int32 ReadyCount = 0;
    int32 TotalCount = 0;

    for (APlayerState* PS : GS->PlayerArray)
    {
        APGPlayerState* PGPS = Cast<APGPlayerState>(PS);
        if (!PGPS)
        {
            continue;
        }

        ++TotalCount;
        if (PGPS->bClientReady)
        {
            ++ReadyCount;
        }
    }

    if (TotalCount < RequiredPlayers)
    {
        return;
    }

    if (ReadyCount < RequiredPlayers)
    {
        return;
    }

    bAllClientsReady = true;
    GetWorldTimerManager().ClearTimer(ReadyTimeoutTimerHandle);

    if (APGGameStateBase* PGGameState = Cast<APGGameStateBase>(GS))
    {
        PGGameState->StartGameTimer();
    }

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (APGPlayerController* PC = Cast<APGPlayerController>(It->Get()))
        {
            PC->Client_AllClientsReady();
        }
    }

}

void APGGameModeBase::StartReadyPolling()
{
    if (bAllClientsReady || bAbortInProgress)
    {
        return;
    }

    if (GetWorldTimerManager().IsTimerActive(ReadyPollTimerHandle))
    {
        return;
    }

    GetWorldTimerManager().SetTimer(
        ReadyPollTimerHandle,
        this,
        &ThisClass::TickReadyPolling,
        1.0f,
        true
    );
}

void APGGameModeBase::StopReadyPolling()
{
    GetWorldTimerManager().ClearTimer(ReadyPollTimerHandle);
}

void APGGameModeBase::TickReadyPolling()
{
    if (GetNumPlayers() < RequiredPlayers)
    {
        return;
    }

    CheckAllClientsReady();

    if (bAllClientsReady || bAbortInProgress)
    {
        StopReadyPolling();
    }
}

void APGGameModeBase::TryStartReadyCountdown()
{
    if (bAllClientsReady || bReadyCountdownStarted || bAbortInProgress)
    {
        return;
    }

    if (GetNumPlayers() < RequiredPlayers)
    {
        return;
    }

    bReadyCountdownStarted = true;
    GetWorldTimerManager().SetTimer(
        ReadyTimeoutTimerHandle,
        this,
        &ThisClass::OnReadyTimeout,
        ReadyTimeoutSeconds,
        false
    );
}

void APGGameModeBase::OnReadyTimeout()
{
    if (bAllClientsReady || bAbortInProgress)
    {
        return;
    }

    bAbortInProgress = true;
    GetWorldTimerManager().ClearTimer(ReadyTimeoutTimerHandle);

    //로비로 이동
}

#pragma region DeathAndRespawn
// 캐릭터 사망 처리
void APGGameModeBase::HandleCharacterDeath(APGPlayerCharacterBase* DeadCharacter, AController* InstigatorController)
{
    if (!DeadCharacter)
    {
        return;
    }


    AController* DeadController = DeadCharacter->GetController();
    APGPlayerState* VictimPS = DeadController ? DeadController->GetPlayerState<APGPlayerState>() : nullptr;
    APGPlayerState* KillerPS = InstigatorController ? InstigatorController->GetPlayerState<APGPlayerState>() : nullptr;

    float RespawnDelay = BaseRespawnTime;
    if (VictimPS)
    {
        RespawnDelay += 5;
    }

    if (DeadController)
    {
        // 🔹 여기: AController* → APGPlayerController* 로 캐스팅
        APGPlayerController* DeadPGController = Cast<APGPlayerController>(DeadController);
        if (DeadPGController)
        {
            // Alive 목록에서 제거
            AlivePlayerControllers.Remove(DeadPGController);

            // Dead 목록에 추가
            DeadPlayerControllers.AddUnique(DeadPGController);
        }

        // 부활 큐 등록
        FPGRespawnInfo Info;
        Info.Controller = DeadPGController;
        Info.RespawnTime = GetWorld()->GetTimeSeconds() + RespawnDelay;

        PendingRespawns.Add(Info);
    }
}

// 주기적으로 부활 대기열을 체크하고 부활 처리
void APGGameModeBase::TickRespawn()
{
    const float CurrentTime = GetWorld()->GetTimeSeconds();

    for (int32 i = PendingRespawns.Num() - 1; i >= 0; --i)
    {
        FPGRespawnInfo& Info = PendingRespawns[i];
        if (!Info.Controller.IsValid())
        {
            PendingRespawns.RemoveAt(i);
            continue;
        }

        if (CurrentTime >= Info.RespawnTime)
        {
            RespawnPlayer(Info.Controller.Get());
            PendingRespawns.RemoveAt(i);
        }
    }
}

// 리스폰 캐릭터 상태 변경
void APGGameModeBase::RespawnPlayer(APGPlayerController* Controller)
{
    if (!Controller)
    {
        return;
    }

    if (APGPlayerCharacterBase* PGChar = Cast<APGPlayerCharacterBase>(Controller->GetPawn()))
    {
        PGChar->SetDeadState(0);
    }

    // 5) Alive / Dead 리스트 갱신
    DeadPlayerControllers.Remove(Controller);
    AlivePlayerControllers.AddUnique(Cast<APGPlayerController>(Controller));
}

#pragma endregion DeathAndRespawn

#pragma region EndGame

void APGGameModeBase::OnObjectiveDestroyed(AActor* DestroyedActor)
{
    APGGameStateBase* GS = GetGameState<APGGameStateBase>();
    if (!GS) 
    {
        return;
    }

    if (!HasAuthority()) 
    {
        return;
    }

    bool bGameOver = true;

    if (DestroyedActor->ActorHasTag("Team1Nexus"))
    {
        GS->TeamResult = ETeamResult::Team2Win;
    }
    else if (DestroyedActor->ActorHasTag("Team2Nexus"))
    {
        GS->TeamResult = ETeamResult::Team1Win;
    }
    else
    {
        bGameOver = false;
    }

    GetWorld()->GetTimerManager().SetTimer(
        EndGameTimerHandle,
        this,
        &APGGameModeBase::ReturnToLobby,
        ReturnLobbyTime,
        false
    );
}

void APGGameModeBase::ReturnToLobby()
{
    UGameInstance* GI = GetGameInstance();
    if (IsValid(GI) == true)
    {
        UConnectSubsystem* ConnectSubsystem = GI->GetSubsystem<UConnectSubsystem>();
        if (IsValid(ConnectSubsystem) == true)
        {
            ConnectSubsystem->TravelToLobby();
        }
    }
}
#pragma endregion EndGame
