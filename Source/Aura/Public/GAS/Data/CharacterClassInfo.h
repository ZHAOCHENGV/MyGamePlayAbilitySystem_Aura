// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ScalableFloat.h"
#include "CharacterClassInfo.generated.h"

class UGameplayAbility;
class UGameplayEffect;

UENUM()
enum class ECharacterClass: uint8
{
	Elementtalist,// 元素法师
	Warrior,// 战士
	Ranger// 游侠
};

USTRUCT()
struct FCharacterClassDefaultInfo
{
	GENERATED_BODY()

	// 定义角色的主要属性，使用 GameplayEffect 类作为模板类型
	UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	TSubclassOf<UGameplayEffect> PrimaryAttributes;

	//启动时能力
	UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	//经验值奖励
	UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	FScalableFloat XPReward = FScalableFloat();
};

/**
 * 
 */
UCLASS()
class AURA_API UCharacterClassInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	// 存储每个职业类型（ECharacterClass）与其默认信息（FCharacterClassDefaultInfo）之间的映射关系
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TMap<ECharacterClass, FCharacterClassDefaultInfo> CharacterClassInformation;

	//主要属性（用于加载本地保存的数据）
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> PrimaryAttributes_SetByCaller;
	
	
	// 公共职业的次要属性（比如耐力、敏捷），适用于所有职业
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> SecondaryAttributes;

	// 持续性的公共职业的次要属性（比如耐力、敏捷），适用于所有职业
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> SecondaryAttributes_Infinite;
	

	// 公共职业的生命值等关键属性，适用于所有职业
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> VitalAttributes;

	//公共能力
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> CommonAbilities;

	//损伤计算系数曲线
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults|Damage")
	TObjectPtr<UCurveTable> DamageCalculationCoefficientes;
	
	// 声明函数：获取对应职业的默认信息
	FCharacterClassDefaultInfo GetClassDefault(ECharacterClass CharacterClass);

	
	
};
