// Copyright (c) 2024 Truman Costello. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MathActor.generated.h"

class UMathPrimitiveComponent;

UCLASS()
class MANIMUNREAL_API AMathActor : public AActor
{
	GENERATED_BODY()
	
public:
	AMathActor();
	
	AMathActor(FLinearColor Color, FString DisplayName, uint8 Dimensions, AMathActor* Target);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	void SetTargetActor(AMathActor* MathActor);

	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category=Mesh)
	TObjectPtr<UMathPrimitiveComponent> MathPrimitiveComponent;

protected:
	TSoftObjectPtr<AMathActor> TargetMathActor;
};
