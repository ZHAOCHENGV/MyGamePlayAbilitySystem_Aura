// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AbilityInfo.generated.h"

class UGameplayAbility;
// 定义一个可在蓝图中使用的结构体，用于存储技能相关的信息
USTRUCT(BlueprintType)
struct FAuraAbilityInfo
{
	GENERATED_BODY()
	// 技能标签（分类/唯一标识），在编辑器和蓝图中可编辑/只读
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag AbilityTag = FGameplayTag();

	// 输入绑定标签（运行时通过代码设置），仅在蓝图中可读
	UPROPERTY( BlueprintReadOnly)
	FGameplayTag InputTag = FGameplayTag();
	
	// 技能状态
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag StatusTag = FGameplayTag();

	//冷却标签
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CooldownTag = FGameplayTag();

	//技能类型
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag AbilityType = FGameplayTag();
	
	// 技能图标资源引用（使用const保证资源安全）
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const UTexture2D> Icon = nullptr;

	// 技能背景材质资源引用
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const UMaterialInterface> BackgroundMaterial = nullptr;

	//技能要求等级
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 LevelRequirement = 1;
	
	//技能能力类
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> Ability;
};


/**
 * 技能信息数据资产类
 * 用于集中存储和管理所有技能的相关信息
 */
UCLASS()
class AURA_API UAbilityInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	// 存储所有技能信息的数组（在数据资产编辑器中配置）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilityInformation")
	TArray<FAuraAbilityInfo> AbilityInformation;

	/**
	  * 通过技能标签查找对应的技能信息
	  * @param AbilityTag 要查找的技能标签
	  * @param bLogNotFound 是否在找不到时记录错误日志（默认不记录）
	  * @return 找到的FAuraAbilityInfo结构体，未找到时返回空结构体
	  */
	FAuraAbilityInfo FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound = false)const;
};
