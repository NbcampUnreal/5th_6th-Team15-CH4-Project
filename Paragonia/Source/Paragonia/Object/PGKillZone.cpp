

#include "PGKillZone.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/PGPlayerCharacterBase.h"
#include "GameFramework/Character.h"
#include "GameplayTag/PGGameplayTags.h" // TAG_Data_Damage_Base 태그 포함
#include "Struct/FObjectAttributeData.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h" 

APGKillZone::APGKillZone()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetBoxExtent(FVector(500.f, 500.f, 100.f));
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &APGKillZone::OnOverlapBegin);
}

void APGKillZone::BeginPlay()
{
	Super::BeginPlay();
}

void APGKillZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

	APGPlayerCharacterBase* PC = Cast<APGPlayerCharacterBase>(OtherActor);
    if (!PC) return;

    if (PC->GetIsDead() == true)
    {
        return;
    }

	UAbilitySystemComponent* TargetASC = PC->GetAbilitySystemComponent();
    if (!TargetASC)
    {
        return;
    }

    float DamageToApply = 0.0f;
    if (KillZoneDataTable && !RowName.IsNone())
    {
        static const FString ContextString(TEXT("KillZone Damage Lookup"));
        FObjectAttributeData* RowData = KillZoneDataTable->FindRow<FObjectAttributeData>(RowName, ContextString);
        if (RowData)
        {
            DamageToApply = RowData->AttackPower;
        }
    }

    if (DamageToApply <= 0.0f || !DamageEffectClass) return;

    FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
    ContextHandle.AddSourceObject(this);

    FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, ContextHandle);

    if (SpecHandle.IsValid())
    {
        SpecHandle.Data->SetSetByCallerMagnitude(TAG_Data_Damage_Base, DamageToApply);
        SpecHandle.Data->SetSetByCallerMagnitude(TAG_Data_Damage_Multiplier, 0.0f);

        TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}