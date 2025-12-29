#include "Character/AI/LtBelicaDrone.h"

#include "AttributeSet/CharacterAttributeSet.h"

#include "AbilitySystemComponent.h"

ALtBelicaDrone::ALtBelicaDrone()
{
}

void ALtBelicaDrone::PossessedBy(AController* NewController)
{
	//if (!IsValid(NewController))
	//{
	//	return;
	//}

	//NewController->Owner = GetOwner();

	//Super::PossessedBy(NewController);


	if (StateTreeComponent)
	{
		StateTreeComponent->StartLogic();
	}

	if (ASC)
	{
		ASC->InitAbilityActorInfo(this, this);

		if (CharacterAttributeSet)
		{
			CharacterAttributeSet->OnHealthChanged.AddDynamic(this, &ALtBelicaDrone::OnHealthChanged);
		}

		if (HasAuthority())
		{
			GrantStartupAbilities();
		}
	}
}

void ALtBelicaDrone::InitAttackData(const FAttackData& InData)
{
	AttackData = InData;
}

FAttackData ALtBelicaDrone::GetAttackData() const
{
	return AttackData;
}

