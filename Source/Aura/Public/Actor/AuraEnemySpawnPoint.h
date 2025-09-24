// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "GAS/Data/CharacterClassInfo.h"
#include "AuraEnemySpawnPoint.generated.h"


class AEnemyCharacter;
/**
 * 
 */
UCLASS()
class AURA_API AAuraEnemySpawnPoint : public ATargetPoint
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void SpawnEnemy();
	
	/**
	 * @brief 定义了这个生成点将要生成的敌人种类。
	 *
	 * TSubclassOf 是一种特殊的智能指针，它提供了一个下拉菜单，
	 * 只能在蓝图编辑器中选择 AEnemyCharacter 或其任何子类，
	 * 这可以防止设计师错误地选择一个非敌人的 Actor 类型。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Class")
	TSubclassOf<AEnemyCharacter> EnemyClass;

	/**
	 * @brief 将要生成的敌人的等级。
	 *
	 * 这个值会在敌人生成后，传递给敌人实例，用于初始化其属性（如生命值、攻击力等）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Class")
	int32 EnemyLevel = 1;

	/**
	 * @brief 将要生成的敌人的职业。
	 *
	 * ECharacterClass 是一个自定义的枚举 (UENUM)，用于定义不同的职业类型（如战士、法师）。
	 * 这个值同样会在生成后传递给敌人实例，可能会影响其技能、行为或外观。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Class")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;
	
};
