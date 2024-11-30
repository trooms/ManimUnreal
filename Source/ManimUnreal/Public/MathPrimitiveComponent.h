// Copyright (c) 2024 Truman Costello. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "MathPrimitiveComponent.generated.h"

/**
 * 
 */
UCLASS()
class MANIMUNREAL_API UMathPrimitiveComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UMathPrimitiveComponent();

	FORCEINLINE void SetDimensions(uint8 NewDimension) { Dimensions = NewDimension; }
	
	UFUNCTION(BlueprintCallable, Category="MathPrimitive")
	void SetColor(FLinearColor NewColor);

	UFUNCTION(BlueprintCallable, Category="MathPrimitive|Transform")
	void Shift(FVector Delta);

	UFUNCTION(BlueprintCallable, Category="MathPrimitive|Transform")
	void Scale(float ScaleFactor, FVector Pivot = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Category="MathPrimitive|Transform")
	void Rotate(FRotator DeltaRotation, FVector Pivot = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Category="MathPrimitive|SubMathComponents")
	void AddPoint(FVector Location);

	UFUNCTION(BlueprintCallable, Category="MathPrimitive|SubMathComponents")
	void AddSubMathComponent(UMathPrimitiveComponent* SubMathComponent);

	UFUNCTION(BlueprintCallable, Category="MathPrimitive|SubMathComponents")
	void RemoveSubMathComponent(UMathPrimitiveComponent* SubMathComponent);

	UFUNCTION(BlueprintCallable, Category="MathPrimitive|SubMathComponents")
	void ClearSubMathComponents();

	void AddUpdater(TFunction<void(float)> UpdaterFunc);
	void ClearUpdaters();

protected:
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	
	virtual bool IsAnySimulatingPhysics() const override { return false; }

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void DestroyComponent(bool bPromoteChildren = false) override;

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual bool ShouldCollideWhenPlacing() const override
	{
		return IsCollisionEnabled();
	}

private:
	uint8 Dimensions = 2;
	FLinearColor Color;
	TArray<TSoftObjectPtr<UMathPrimitiveComponent>> SubMathComponents;
	TArray<FVector> Points;
	TArray<TFunction<void(float)>> Updaters;

	TWeakObjectPtr<UMathPrimitiveComponent> ParentMathComponent;
};
