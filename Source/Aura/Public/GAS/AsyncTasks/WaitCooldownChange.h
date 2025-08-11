// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "ActiveGameplayEffectHandle.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "WaitCooldownChange.generated.h"


class UAbilitySystemComponent;
// 定义一个动态多播委托，用于通知冷却时间变化
// 参数：TimeRemaining - 剩余的冷却时间（以秒为单位）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCooldownChangeSignature, float, TimeRemaining);

/**
 * 异步任务类：用于监听技能冷却时间的变化，并在冷却开始或结束时触发事件。
 * 通过 UAbilitySystemComponent 监听 GameplayTag 的变化。
 */
UCLASS(BlueprintType, meta = (ExposedAsyncProxy = "AsyncTask"))
class AURA_API UWaitCooldownChange : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	// 当冷却开始时触发的委托，会广播剩余的冷却时间
	UPROPERTY(BlueprintAssignable)
	FCooldownChangeSignature CooldownStart;

	// 当冷却结束时触发的委托，会广播0.0秒的剩余时间
	UPROPERTY(BlueprintAssignable)
	FCooldownChangeSignature CooldownEnd;

	/**
	 * 创建并初始化一个WaitCooldownChange异步任务
	 * @param AbilitySystemComponent - 要监听的能力系统组件
	 * @param InCooldownTag - 要监听的冷却标签
	 * @return 创建的WaitCooldownChange实例，如果参数无效则返回nullptr
	 */
	UFUNCTION(BlueprintCallable , meta = (BlueprintInternalUseOnly = "true"))
	static UWaitCooldownChange* WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent,const FGameplayTag& InCooldownTag);

	/**
	 * 手动结束任务，清理所有注册的委托并标记对象可被回收
	 */
	UFUNCTION(BlueprintCallable)
	void EndTask();

protected:
	// 持有对能力系统组件的引用
	TObjectPtr<UAbilitySystemComponent> ASC;

	// 要监听的冷却标签
	FGameplayTag CooldownTag;

	/**
	 * 当监听的冷却标签发生变化时调用
	* @param InCooldownTag - 变化的标签
	 * @param NewCount - 标签的新计数，0表示标签被移除
	*/
	void CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount);

	/**
	 * 当新的游戏效果被添加到能力系统组件时调用
	 * 用于检测是否有冷却效果被应用，并广播剩余时间
	 * @param TargetASC - 接收效果的能力系统组件
	 * @param SpecApplied - 应用的效果规格
	 * @param ActiveEffectHandle - 活动效果的句柄
	 */
	void OnActiveEffectAdded(UAbilitySystemComponent* TargetASC,const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveEffectHandle);
};
