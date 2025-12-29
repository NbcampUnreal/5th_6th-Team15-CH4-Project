// Fill out your copyright notice in the Description page of Project Settings.


#include "WaveSystem/WaveSpawner.h"
#include "GameState/PGGameStateBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/AI/MinionCharacter.h"
#include "Kismet/GameplayStatics.h"

AWaveSpawner::AWaveSpawner()
	: CurrentSpawnIndex(0),
	SpawnRate(30)
{
	// Server에서만 사용 예정
	bReplicates = false;
	bNetLoadOnClient = false;

	PathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PathSpline"));
	RootComponent = PathSpline;
}

void AWaveSpawner::BeginPlay()
{
	Super::BeginPlay();

	CacheDataTableTags();

	SpawnNames.Reserve(30);

	// 서버용이지만 추가 체크
	if (HasAuthority())
	{
		APGGameStateBase* GameState = GetNowWorldGameState();
		if (IsValid(GameState))
		{
			if (GameState->OnGameTimeUpdated.IsAlreadyBound(this, &AWaveSpawner::HandleGameTimeUpdate) == false)
			{
				GameState->OnGameTimeUpdated.AddDynamic(this, &AWaveSpawner::HandleGameTimeUpdate);
			}
		}
	}
}

void AWaveSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		APGGameStateBase* GameState = GetNowWorldGameState();
		if (IsValid(GameState))
		{
			GameState->OnGameTimeUpdated.RemoveDynamic(this, &AWaveSpawner::HandleGameTimeUpdate);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AWaveSpawner::OnWaveStart()
{
	if (IsValid(CurrentWaveDefinition) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("AWaveSpawner::OnWaveStart - CurrentWaveDefinition is not Valid!"));
		return;
	}

	SpawnNames.Empty();
	CurrentSpawnIndex = 0;

	APGGameStateBase* GameState = GetNowWorldGameState();
	if (IsValid(GameState) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("AWaveSpawner::OnWaveStart - APGGameStateBase is not Valid!"));
		return;
	}

	for (const FWaveSpawnInfo& Info : CurrentWaveDefinition->WaveSpawnList)
	{
		if (GameState->GetCurrentGameTime() < Info.StartSpawnTime)
			continue;

		if (const FName* FoundRowName = TagToRowMap.Find(Info.SpawnNpcTag))
		{
			for (int32 i = 0; i < Info.SpawnCount; i++)
			{
				SpawnNames.Add(*FoundRowName);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("WaveSpawner::OnWaveStart - Tag [%s] not found in DataTable!"), *Info.SpawnNpcTag.ToString());
		}
	}

	if (SpawnNames.Num() > 0)
	{
		float Rate = CurrentWaveDefinition->SpawnInterval;

		if (Rate <= 0.0f) 
			Rate = 0.5f;

		GetWorldTimerManager().SetTimer(
			SpawnTimerHandle, this, &AWaveSpawner::HandleSpawnTick, Rate, true, 0.0f);
	}
}

void AWaveSpawner::HandleGameTimeUpdate(int32 CurrentTime)
{
	if (CurrentTime % SpawnRate == 0)
	{
		OnWaveStart();
	}
}

void AWaveSpawner::CacheDataTableTags()
{
	if (IsValid(WaveDataTable) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("AWaveSpawner::CacheDataTableTags - DataTable is not Valid!"));
		return;
	}

	TagToRowMap.Empty();

	TArray<FName> RowNames = WaveDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		FNpcDataRow* Row = WaveDataTable->FindRow<FNpcDataRow>(RowName,"");

		if (Row != nullptr && 
			Row->NPCTypeTag.IsValid())
		{
			TagToRowMap.Add(Row->NPCTypeTag, RowName);
		}
	}
}

void AWaveSpawner::HandleSpawnTick()
{
	if (CurrentSpawnIndex >= SpawnNames.Num())
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}

	FName RowToSpawn = SpawnNames[CurrentSpawnIndex];

	CurrentSpawnIndex++;

	SpawnMinion(RowToSpawn);
}

void AWaveSpawner::SpawnMinion(FName RowName)
{
	if (IsValid(WaveDataTable) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("AWaveSpawner::SpawnMinion - DataTable is not Valid!"));
		return;
	}

	static const FString ContextString(TEXT("SpawnMinion"));
	FNpcDataRow* RowData = WaveDataTable->FindRow<FNpcDataRow>(RowName, ContextString);

	if (RowData == nullptr
		|| IsValid(RowData->SpawnNpcClass) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("AWaveSpawner::SpawnMinion - RowData is Weird!"));
		return;
	}

	FVector Loc = PathSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
	FRotator Rot = PathSpline->GetRotationAtSplinePoint(0, ESplineCoordinateSpace::World);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FTransform SpawnTransform(Rot, Loc);
	AMinionCharacter* NewMinion = GetWorld()->SpawnActorDeferred<AMinionCharacter>(
		RowData->SpawnNpcClass,
		SpawnTransform,
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
	);

	if (NewMinion != nullptr)
	{
		NewMinion->SetTeamId(TeamId);
		NewMinion->SetMovementSpline(PathSpline); 
		NewMinion->SetAttackRange(RowData->AttackRange);
		NewMinion->SetSightRange(RowData->SightRange);

		UGameplayStatics::FinishSpawningActor(NewMinion, SpawnTransform);

		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(NewMinion);
		if (ASC != nullptr &&
			IsValid(MinionInitEffectClass))
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(MinionInitEffectClass, 1.0f, Context);

			if (SpecHandle.IsValid())
			{
				// 차후 GameState->GameTime 등으로 변경 가능
				float CurrentTime = 0; 
				float GameMinutes = CurrentTime / 60.0f;

				float ScaleFactor = 1.0f + (GameMinutes * 0.05f);
				ScaleFactor = FMath::Clamp(ScaleFactor, 1.0f, 2.0f);

				float FinalHealth = RowData->BaseHealth * ScaleFactor;
				float FinalAttack = RowData->BaseAttack * ScaleFactor;
				float FinalDefense = RowData->BaseDefense * ScaleFactor;

				// GE 에서 해당 태그에 맞추어 CharacterAttributeSet에 넣어줄 예정
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("AI.NPC.Stat.Health")), FinalHealth);
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("AI.NPC.Stat.Attack")), FinalAttack);
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("AI.NPC.Stat.Defense")), FinalDefense);
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("AI.NPC.Stat.Speed")), RowData->MoveSpeed);

				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}

APGGameStateBase* AWaveSpawner::GetNowWorldGameState()
{
	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		return World->GetGameState<APGGameStateBase>();
	}
	return nullptr;
}
