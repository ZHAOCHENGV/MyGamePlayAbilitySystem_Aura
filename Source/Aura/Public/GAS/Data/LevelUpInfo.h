// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelUpInfo.generated.h"

USTRUCT(BlueprintType)
struct FAuraLevelUpInfo
{
	GENERATED_BODY()

	//升级要求
	UPROPERTY(EditDefaultsOnly)
	int32 LevelUpRequirement = 0;
	//属性点奖励
	UPROPERTY(EditDefaultsOnly)
	int32 AttributePointAward = 1;
	//法术点奖励
	UPROPERTY(EditDefaultsOnly)
	int32 SpellPointAward = 1;
};


/**
 * 
 */


UCLASS()
class AURA_API ULevelUpInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	//升级信息
	UPROPERTY(EditDefaultsOnly)
	TArray<FAuraLevelUpInfo> LevelUpInformation;
	//查找 XP经验值 等级
	int32 FindLevelForXp(int32 XP)const;
};
