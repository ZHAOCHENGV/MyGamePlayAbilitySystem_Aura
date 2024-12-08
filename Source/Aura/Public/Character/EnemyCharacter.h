// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "Interation/EnemyInterface.h"
#include "EnemyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AEnemyCharacter : public ACharacterBase,public IEnemyInterface
{
	GENERATED_BODY()
public:
	AEnemyCharacter();
	//重载接口的 突出演员 事件
	virtual void HigHlihtActor()override ;
	//重载接口的 不突出演员 事件
	virtual void UnHigHlightActor()override;
protected:
	virtual void BeginPlay() override;
	
	//初始化 能力Actor信息集
	virtual void InitAbilityActorInfo() override;
	
};
