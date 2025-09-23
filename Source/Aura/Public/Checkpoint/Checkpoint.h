// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Aura/Aura.h"
#include "GameFramework/PlayerStart.h"
#include "Interation/HighlightInterface.h"
#include "Interation/SaveInterface.h"
#include "Checkpoint.generated.h"

class USphereComponent;
/**
 * 
 */
UCLASS()
class AURA_API ACheckpoint : public APlayerStart, public ISaveInterface, public IHighlightInterface
{
	GENERATED_BODY()

	
public:
	ACheckpoint(const FObjectInitializer& ObjectInitializer);

	/* Start Save Interface*/
	virtual bool ShouldLoadTransform_Implementation() override {return false; };
	virtual void LoadActor_Implementation() override;
	/* End Save Interface*/

	/* Start HighlightInterface*/
	virtual void HighlightActor_Implementation() override;
	virtual void UnHighlightActor_Implementation() override;
	virtual void SetMoveToLocation_Implementation(FVector& OutDestination) override;
	/* End HighlightInterface*/

	//是否已到达
	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool bReached = false;

	//是否绑定重叠回调
	UPROPERTY(EditAnywhere)
	bool bBindOverlapCallback = true;
protected:
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void BeginPlay() override;

	//已到达检查点
	UFUNCTION(BlueprintImplementableEvent)
	void CheckpointReached(UMaterialInstanceDynamic* DynamicMaterialInstance);

	//处理发光效果
	UFUNCTION(BlueprintCallable)
	void HandleGlowEffects();


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> CheckpointMesh;

	UPROPERTY(EditDefaultsOnly)
	int32 CustomDepthStencilOverride = CUSTOM_DEPTH_TAN;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> MoveToComponent;


	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;
};
