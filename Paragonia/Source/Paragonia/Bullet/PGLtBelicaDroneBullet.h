#pragma once

#include "CoreMinimal.h"
#include "Bullet/PGCreateTargetActorBullet.h"
#include "PGLtBelicaDroneBullet.generated.h"

UCLASS()
class PARAGONIA_API APGLtBelicaDroneBullet : public APGCreateTargetActorBullet
{
	GENERATED_BODY()
public:
	APGLtBelicaDroneBullet();

	void ProcessHitCheck();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SceneRoot;
};
