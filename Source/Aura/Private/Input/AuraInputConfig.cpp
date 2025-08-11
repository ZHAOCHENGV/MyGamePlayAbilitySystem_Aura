// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/AuraInputConfig.h"

const UInputAction* UAuraInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag,bool bLogNotFound) const
{
	// 遍历 AbilityInputActions 列表，该列表包含所有能力输入的绑定关系
	for (const FAuraInputAction&Action : AbilityInputActions)
	{
		// 检查当前 Action 是否有效，并且其 InputTag 是否与目标 InputTag 匹配
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			// 如果找到对应的输入动作，直接返回指针
			return Action.InputAction;
		}
	}
	// 如果未找到对应的输入动作，并且 bLogNotFound 为 true，则记录错误日志
	if (bLogNotFound)
	{
		
		UE_LOG(LogTemp,Error,TEXT("在输入配置[%s]中，没有找到输入操作[%s]."),*GetNameSafe(this),*InputTag.ToString());

	}
	// 如果未找到匹配的输入动作，返回 nullptr
	return nullptr;
}
