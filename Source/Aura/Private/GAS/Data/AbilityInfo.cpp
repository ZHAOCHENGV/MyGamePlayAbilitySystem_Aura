// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Data/AbilityInfo.h"

#include "Aura/AuraLogChannels.h"

FAuraAbilityInfo UAbilityInfo::FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound) const
{
	// 遍历所有存储的技能信息
	for (const FAuraAbilityInfo& Info : AbilityInformation)
	{
		// 当找到匹配的技能标签时
		if (Info.AbilityTag == AbilityTag)
		{
			// 返回对应的技能信息
			return Info;
		}
	}
	// 未找到时的处理
	if (bLogNotFound)
	{
		// 记录详细错误日志（包含标签和资源名称）
		UE_LOG(LogAura, Error, TEXT("Can't find info for AbilityTag [%s] on AbilityInfo [%s]"),*AbilityTag.ToString(),*GetNameSafe(this));
	}
	// 返回一个空的默认结构体（而不是指针，避免空指针异常）
	return FAuraAbilityInfo();
}
