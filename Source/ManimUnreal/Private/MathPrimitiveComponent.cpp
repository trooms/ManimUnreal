// Copyright (c) 2024 Truman Costello. All rights reserved.


#include "MathPrimitiveComponent.h"

UMathPrimitiveComponent::UMathPrimitiveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	Dimensions = 2;
	Color = FLinearColor::White;
}

void UMathPrimitiveComponent::SetColor(FLinearColor NewColor)
{
	Color = NewColor;
	MarkRenderStateDirty();
}

void UMathPrimitiveComponent::Shift(FVector Delta)
{
	for (FVector& Point : Points)
	{
		Point += Delta;
	}
	for (TSoftObjectPtr<UMathPrimitiveComponent> SubComponent : SubMathComponents)
	{
		if (SubComponent.Get())
		{
			SubComponent->Shift(Delta);
		}
	}
	MarkRenderStateDirty();
}

void UMathPrimitiveComponent::Scale(float ScaleFactor, FVector Pivot)
{
	for (FVector& Point : Points)
	{
		Point = Pivot + (Point - Pivot) * ScaleFactor;
	}

	for (TSoftObjectPtr<UMathPrimitiveComponent> SubComponent : SubMathComponents)
	{
		if (SubComponent.Get())
		{
			SubComponent->Scale(ScaleFactor, Pivot);
		}
	}
	MarkRenderStateDirty();
}

void UMathPrimitiveComponent::Rotate(FRotator DeltaRotation, FVector Pivot)
{
	FQuat Quat = FQuat(DeltaRotation);
	for (FVector& Point : Points)
	{
		Point = Pivot + Quat.RotateVector(Point - Pivot);
	}

	for (TSoftObjectPtr<UMathPrimitiveComponent> SubComponent : SubMathComponents)
	{
		if (SubComponent.Get())
		{
			SubComponent->Rotate(DeltaRotation, Pivot);
		}
	}

	MarkRenderStateDirty();
}

void UMathPrimitiveComponent::AddPoint(FVector Location)
{
	Points.Add(Location);
}

void UMathPrimitiveComponent::AddSubMathComponent(UMathPrimitiveComponent* SubMathComponent)
{
	if (SubMathComponent && !SubMathComponents.Contains(SubMathComponent))
	{
		SubMathComponents.Add(SubMathComponent);
		SubMathComponent->SetupAttachment(this);
		SubMathComponent->RegisterComponent();
		MarkRenderStateDirty();
	}
}

void UMathPrimitiveComponent::RemoveSubMathComponent(UMathPrimitiveComponent* SubMathComponent)
{
	if (SubMathComponent && !SubMathComponents.Contains(SubMathComponent))
	{
		SubMathComponents.Remove(SubMathComponent);
		MarkRenderStateDirty();
	}
}

void UMathPrimitiveComponent::ClearSubMathComponents()
{
	for (TSoftObjectPtr<UMathPrimitiveComponent> SubComponent : SubMathComponents)
	{
		if (SubComponent.Get())
		{
			SubComponent->DestroyComponent();
		}
	}
	SubMathComponents.Empty();
	MarkRenderStateDirty();
}

void UMathPrimitiveComponent::AddUpdater(TFunction<void(float)> UpdaterFunc)
{
	Updaters.Add(UpdaterFunc);
}

void UMathPrimitiveComponent::ClearUpdaters()
{
	Updaters.Empty();
}

FPrimitiveSceneProxy* UMathPrimitiveComponent::CreateSceneProxy()
{
    // Inner class defining the scene proxy
    class FMathPrimitiveSceneProxy final : public FPrimitiveSceneProxy
    {
    public:
        FMathPrimitiveSceneProxy(const UMathPrimitiveComponent* InComponent)
            : FPrimitiveSceneProxy(InComponent)
            , bDrawOnlyIfSelected(false)
            , Color(InComponent->Color)
            , LineThickness(1.0f)
            , Points(InComponent->Points)
            , bLoop(false)
        {
            bWillEverBeLit = true;
        }
    	
        virtual SIZE_T GetTypeHash() const override
        {
            static size_t UniquePointer;
            return reinterpret_cast<size_t>(&UniquePointer);
        }
    	
        virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views,
                                            const FSceneViewFamily& ViewFamily,
                                            uint32 VisibilityMap,
                                            FMeshElementCollector& Collector) const override
        {
            QUICK_SCOPE_CYCLE_COUNTER(STAT_MathPrimitiveSceneProxy_GetDynamicMeshElements);

            for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
            {
                if (VisibilityMap & (1 << ViewIndex))
                {
                    const FSceneView* View = Views[ViewIndex];
                    FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

                    const FMatrix& LocalToWorld = GetLocalToWorld();
                    const FLinearColor DrawColor = GetViewSelectionColor(Color, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected());

                    // Taking into account the min and maximum drawing distance
                    const float DistanceSqr = (View->ViewMatrices.GetViewOrigin() - LocalToWorld.GetOrigin()).SizeSquared();
                    if (DistanceSqr < FMath::Square(GetMinDrawDistance()) || DistanceSqr > FMath::Square(GetMaxDrawDistance()))
                    {
                        continue;
                    }
                	
                    if (Points.Num() >= 2)
                    {
                        for (int32 i = 0; i < Points.Num() - 1; i++)
                        {
                            FVector Start = LocalToWorld.TransformPosition(Points[i]);
                            FVector End = LocalToWorld.TransformPosition(Points[i + 1]);
                            PDI->DrawLine(Start, End, DrawColor, SDPG_World, LineThickness);
                        }
                    	
                        if (bLoop)
                        {
                            FVector Start = LocalToWorld.TransformPosition(Points.Last());
                            FVector End = LocalToWorld.TransformPosition(Points[0]);
                            PDI->DrawLine(Start, End, DrawColor, SDPG_World, LineThickness);
                        }
                    }

                    // TODO:
                    // ender filled triangles or other geometry
                    // For example, render as a simple mesh if Points define a convex polygon
                    // This requires more complex implementation using vertex buffers and index buffers
                }
            }
        }
    	
        virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
        {
            FPrimitiveViewRelevance Result;
            Result.bDrawRelevance = IsShown(View) && (!bDrawOnlyIfSelected || IsSelected());
            Result.bDynamicRelevance = true;
            Result.bShadowRelevance = IsShadowCast(View);
            Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
            return Result;
        }
    	
        virtual uint32 GetMemoryFootprint() const override { return sizeof(*this) + GetAllocatedSize(); }
        uint32 GetAllocatedSize() const { return FPrimitiveSceneProxy::GetAllocatedSize(); }

    private:
        const uint32 bDrawOnlyIfSelected : 1;
        const FLinearColor Color;
        const float LineThickness;
    	
        TArray<FVector> Points;
        bool bLoop;
    };

    return new FMathPrimitiveSceneProxy(this);
}

void UMathPrimitiveComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (auto& Updater : Updaters)
	{
		Updater(DeltaTime);
	}
	MarkRenderStateDirty();
}

void UMathPrimitiveComponent::DestroyComponent(bool bPromoteChildren)
{
	ParentMathComponent->RemoveSubMathComponent(this);
	Super::DestroyComponent(bPromoteChildren);
}

FBoxSphereBounds UMathPrimitiveComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	if (Points.Num() == 0)
	{
		return FBoxSphereBounds(FVector::ZeroVector, FVector::ZeroVector, 0.0f);
	}
	FBox BoundingBox(Points[0], Points[0]);
	
	for (const FVector& Point : Points)
	{
		BoundingBox += Point;
	}
	return FBoxSphereBounds(BoundingBox).TransformBy(LocalToWorld);
}

