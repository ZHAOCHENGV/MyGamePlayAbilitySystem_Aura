// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Checkpoint/Checkpoint.h"
#include "MapEntrance.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AMapEntrance : public ACheckpoint
{
	GENERATED_BODY()
public:

	AMapEntrance(const FObjectInitializer& ObjectInitializer);
	
	/* Start Save Interface*/
	virtual void LoadActor_Implementation() override;
	/* End Save Interface*/

	
	/* Start HighlightInterface*/
	virtual void HighlightActor_Implementation() override;
	/* End HighlightInterface*/

	//目标地图
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> DestinationMap;

	//出生地
	UPROPERTY(EditAnywhere)
	FName DestinationPlayerStartTag;
protected:

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
