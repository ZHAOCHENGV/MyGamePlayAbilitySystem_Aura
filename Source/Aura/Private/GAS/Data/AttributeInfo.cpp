// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Data/AttributeInfo.h"

FAuraAttributeInfo UAttributeInfo::FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound)
{
	// 遍历所有的属性信息
	for (const FAuraAttributeInfo &Info : AttributeInformation )
	{
		// 检查当前属性信息的标签是否与传入的标签完全匹配
		// 这里也可以用 Info.AttributeTag == AttributeTag 进行比较
		// 使用MatchesTagExact，是为了确保标签完全匹配
		if (Info.AttributeTag.MatchesTagExact(AttributeTag))
		{
			// 找到匹配的属性标签，返回该属性信息
			return Info;
		}
		
	}
	// 如果没有找到匹配的属性信息，并且需要记录未找到情况
	if (bLogNotFound)
	{
		// 记录错误日志，说明在当前对象的属性信息列表中没有找到给定的标签
		UE_LOG(LogTemp,Error,TEXT("在FAuraAttributeInfo[%s]中，没有找到属性标签[%s]."),*GetNameSafe(this),*AttributeTag.ToString());
	}
	// 返回一个默认构造的 FAuraAttributeInfo 对象，表示未找到匹配项
	return FAuraAttributeInfo();
}


