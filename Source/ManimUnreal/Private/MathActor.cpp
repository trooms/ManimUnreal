// Copyright (c) 2024 Truman Costello. All rights reserved.


#include "MathActor.h"

#include "MathPrimitiveComponent.h"

AMathActor::AMathActor()
{
	PrimaryActorTick.bCanEverTick = true;
	MathPrimitiveComponent = CreateDefaultSubobject<UMathPrimitiveComponent>(TEXT("MathPrimitiveComponent"));
	RootComponent = MathPrimitiveComponent;
}

AMathActor::AMathActor(FLinearColor Color, FString DisplayName, uint8 Dimensions, AMathActor* Target)
	: TargetMathActor(Target)
{
	MathPrimitiveComponent = CreateDefaultSubobject<UMathPrimitiveComponent>(FName(*DisplayName));
	MathPrimitiveComponent->SetDimensions(Dimensions);
	MathPrimitiveComponent->SetColor(Color);

	SetActorLabel(DisplayName);
}

// Called when the game starts or when spawned
void AMathActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMathActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (TargetMathActor.IsValid())
	{
		FVector TargetLocation = TargetMathActor->GetActorLocation();
		FVector CurrentLocation = GetActorLocation();
		FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, 2.0f);
		SetActorLocation(NewLocation);
	}
}

void AMathActor::SetTargetActor(AMathActor* MathActor)
{
	TargetMathActor = MathActor;
}

