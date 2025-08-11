// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AuraInputConfig.generated.h"

USTRUCT()
struct FAuraInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	const class UInputAction* InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag();

};

/**
 * 
 */


UCLASS()
class AURA_API UAuraInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	// 在输入配置中查找与指定 GameplayTag 对应的输入动作
	// 参数:
	// InputTag: 要查找的输入标签 (GameplayTag 类型)
	// bLogNotFound: 是否记录未找到输入动作的错误日志
	// 返回值:
	// 如果找到对应的输入动作 (UInputAction)，则返回指向它的指针；如果未找到，则返回 nullptr。
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag,bool bLogNotFound = false) const;
	
	UPROPERTY(EditDefaultsOnly)
	TArray<FAuraInputAction> AbilityInputActions;

};
