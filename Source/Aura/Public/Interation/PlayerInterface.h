// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AURA_API IPlayerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	//查找 XP 的级别
	UFUNCTION(BlueprintNativeEvent)
	int32 FindLevelForXP(int32 InXP);
	
	
	//获取经验值
	UFUNCTION(BlueprintNativeEvent)
	int32 GetXP()const;

	//获取属性点奖励
	UFUNCTION(BlueprintNativeEvent)
	int32 GetAttributePointsReward(int32 Level)const;

	//获取法术点奖励
	UFUNCTION(BlueprintNativeEvent)
	int32 GetSpellPointsReward(int32 Level)const;

	
	//添加经验值
	UFUNCTION(BlueprintNativeEvent)
	void AddToXP(int32 InXP);

	//添加到玩家级别
	UFUNCTION(BlueprintNativeEvent)
	void AddToPlayerLevel(int32 InPlayerLevel);

	//添加到属性点
	UFUNCTION(BlueprintNativeEvent)
	void AddToAttributePoints(int32 InAttributePoints);

	//获取属性点
	UFUNCTION(BlueprintNativeEvent)
	int32 GetAttributePoints() const;
	
	//添加到法术点数
	UFUNCTION(BlueprintNativeEvent)
	void AddToSpellPoints(int32 InSpellPoints);
	
	//获取法术点
	UFUNCTION(BlueprintNativeEvent)
	int32 GetSpellPoints() const;
	
	//升級
	UFUNCTION(BlueprintNativeEvent)
	void LeveUp();
};
