// Fill out your copyright notice in the Description page of Project Settings.


#include "PGObject.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet/CharacterAttributeSet.h"
#include "Subsystem/PGAttributeDataSubsystem.h"
#include "Struct/FCharacterAttributeData.h"
#include "Net/UnrealNetwork.h"


// Sets default values
APGObject::APGObject()
{
	PrimaryActorTick.bCanEverTick = false;

	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	ObjectAttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("CharacterAttributeSet"));
}

UAbilitySystemComponent* APGObject::GetAbilitySystemComponent() const
{
	return ASC;
}

// Called when the game starts or when spawned
void APGObject::BeginPlay()
{
	Super::BeginPlay();

	InitializeActorInfo();

	if (HasAuthority())
	{
		InitializeAttributes();
		InitializeAttributesData();
	}
}

void APGObject::InitializeActorInfo()
{
	ASC->InitAbilityActorInfo(this, this);
}

void APGObject::InitializeAttributes()
{
	if (!IsValid(ASC))
	{
		UE_LOG(LogTemp, Warning, TEXT("APGObject::InitializeAttributes - ASC is not valid"));
		return;
	}

	ASC->AddAttributeSetSubobject<UCharacterAttributeSet>(ObjectAttributeSet);

	BindAttributeChangeDelegates();
}

void APGObject::InitializeAttributesData()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(ObjectAttributeSet))
	{
		UE_LOG(LogTemp, Warning, TEXT("APGObject::InitializeAttributesData - ObjectAttributeSet is not valid"));
		return;
	}

	UPGAttributeDataSubsystem* AttributeSubsystem = GetGameInstance()->GetSubsystem<UPGAttributeDataSubsystem>();
	if (!IsValid(AttributeSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("APGObject::InitializeAttributesData - AttributeSubsystem is not valid"));
		return;
	}

	const FCharacterAttributeData* AttributeData = AttributeSubsystem->GetAttributeDataByName(ObjectName);
	if (AttributeData)
	{
		ObjectAttributeSet->InitMaxHealth(AttributeData->MaxHealth);
		ObjectAttributeSet->InitHealth(AttributeData->Health);
		ObjectAttributeSet->InitDefense(AttributeData->Defense);
		ObjectAttributeSet->InitAttackPower(AttributeData->AttackPower);
		ObjectAttributeSet->InitMoveSpeed(AttributeData->MoveSpeed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("APGObject::InitializeAttributesData - No AttributeData found for %s"), *ObjectName.ToString());
	}
}

void APGObject::BindAttributeChangeDelegates()
{
	if (!IsValid(ASC))
	{
		return;
	}

	ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetDefenseAttribute()).AddUObject(this, &ThisClass::OnDefenceChanged);
}

void APGObject::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0.0f)
	{
		HandleHealthDepleted();
	}
}

void APGObject::OnDefenceChanged(const FOnAttributeChangeData& Data)
{
}

void APGObject::HandleHealthDepleted()
{
	if (HasAuthority())
	{
		Destroy();
	}
}

void APGObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGObject, TeamID);
	DOREPLIFETIME(APGObject, bIsDead);
}