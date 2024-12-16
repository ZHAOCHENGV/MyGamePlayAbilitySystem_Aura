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
/** Enemy 接口函数*/
	//重载接口的 突出演员 事件
	virtual void HigHlihtActor()override ;
	//重载接口的 不突出演员 事件
	virtual void UnHigHlightActor()override;
/** 结束Enemy 接口函数*/

/** Combat 接口函数*/
	virtual int32 GetPlayerLevel()override;
/** 结束Combat 接口函数*/
protected:
	virtual void BeginPlay() override;
	//初始化 能力Actor信息集
	virtual void InitAbilityActorInfo() override;

	//等级
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character Class Defaults")
	int32 Level= 1;
	
};
