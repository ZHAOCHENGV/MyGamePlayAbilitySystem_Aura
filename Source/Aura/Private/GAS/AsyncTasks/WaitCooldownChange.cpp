// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AsyncTasks/WaitCooldownChange.h"
#include "AbilitySystemComponent.h"

UWaitCooldownChange* UWaitCooldownChange::WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent,const FGameplayTag& InCooldownTag)
{
	// 创建一个新的WaitCooldownChange对象
	UWaitCooldownChange* WaitCooldownChange = NewObject<UWaitCooldownChange>();
	// 保存传入的能力系统组件引用
	WaitCooldownChange->ASC = AbilitySystemComponent;
	// 保存传入的冷却标签
	WaitCooldownChange->CooldownTag = InCooldownTag;
	// 验证输入参数的有效性
	if (!IsValid(AbilitySystemComponent) || !InCooldownTag.IsValid())
	{
		// 如果参数无效，结束任务并返回nullptr
		WaitCooldownChange->EndTask();
		return nullptr;
	}
	// 注册对冷却标签变化的监听（当标签被添加或移除时）
	AbilitySystemComponent->RegisterGameplayTagEvent(InCooldownTag,EGameplayTagEventType::NewOrRemoved).AddUObject(WaitCooldownChange,&UWaitCooldownChange::CooldownTagChanged);
	// 注册对新增游戏效果的监听，用于检测冷却效果的添加
	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(WaitCooldownChange,&UWaitCooldownChange::OnActiveEffectAdded);
	
	return WaitCooldownChange;
}

void UWaitCooldownChange::EndTask()
{
	// 如果能力系统组件无效，直接返回
	if (!IsValid(ASC))return;
	// 移除对冷却标签事件的所有监听
	ASC->RegisterGameplayTagEvent(CooldownTag,EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
	// 标记对象准备销毁
	SetReadyToDestroy();
	// 标记对象为垃圾，使其被垃圾收集器回收
	MarkAsGarbage();
}

void UWaitCooldownChange::CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount)
{
	// 当标签计数变为0时，意味着冷却结束
	if (NewCount == 0)
	{
		// 广播冷却结束事件，剩余时间为0
		CooldownEnd.Broadcast(0.f);
	}
}

void UWaitCooldownChange::OnActiveEffectAdded(UAbilitySystemComponent* TargetASC,
	const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	// 获取效果中的资产标签
	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags);
	// 获取效果中的已授予资产标签
	FGameplayTagContainer GrantedTags;
	SpecApplied.GetAllGrantedTags(GrantedTags);

	// 检查标签是否匹配监听的冷却标签
	if (AssetTags.HasTagExact(CooldownTag) || GrantedTags.HasTagExact(CooldownTag))
	{
		// 创建查询，查找所有包含冷却标签的活动效果
		FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());
		// 获取这些效果的剩余时间数组
		TArray<float>TimesRemaining= ASC->GetActiveEffectsTimeRemaining(GameplayEffectQuery);
		// 如果有找到匹配的效果
		if (TimesRemaining.Num() > 0)
		{
			// 初始化最大剩余时间为第一个效果的时间
			float TimeRemaining = TimesRemaining[0];
			// 遍历所有效果，找出最长的剩余时间
			for (int32 i = 0; i < TimesRemaining.Num(); i++)
			{
				if (TimesRemaining[i] > TimeRemaining)
				{
					TimeRemaining = TimesRemaining[i] ;
				}
			}
			// 广播冷却开始事件，带上最长的剩余时间
			CooldownStart.Broadcast(TimeRemaining);
		}
		
	}
}
