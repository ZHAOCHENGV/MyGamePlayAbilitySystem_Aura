// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */

class UBehaviorTreeComponent;
UCLASS()
class AURA_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	AEnemyAIController();

public:
protected:

	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;
};
