#include "Bullet/PGLtBelicaDroneBullet.h"

APGLtBelicaDroneBullet::APGLtBelicaDroneBullet()
	: APGCreateTargetActorBullet()
{
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);
}

void APGLtBelicaDroneBullet::ProcessHitCheck()
{
	if (!HasAuthority()) return;

	if (!IsValid(Owner))
	{
		UE_LOG(LogTemp, Warning, TEXT("APGLtBelicaDroneBullet::ProcessHitCheck - Invalid Owner"));
		Destroy();
	}

	HitCheckNotify();
	SetLifeSpan(0.1f);
}
