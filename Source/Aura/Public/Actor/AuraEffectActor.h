// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AuraEffectActor.generated.h"


class UGameplayEffect;
class USphereComponent;
class UStaticMeshComponent;
UCLASS()
class AURA_API AAuraEffectActor : public AActor
{
	GENERATED_BODY()
	
public:	

	
protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere,Category="Applied Effects")
	TSubclassOf<UGameplayEffect> InstantGameplayEffectClass;

	//应用效果到目标
	UPROPERTY(BlueprintCallable)
	void ApplyEffectToTarget(AActor * Target,TSubclassOf<UGameplayEffect> GameplayEffectClass);
	
	
};