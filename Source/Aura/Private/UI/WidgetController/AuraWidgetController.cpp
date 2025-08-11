// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/AuraWidgetController.h"

#include "GAS/AuraAbilitySystemComponent.h"
#include "GAS/AuraAttributeSet.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "GAS/Data/AbilityInfo.h"

void UAuraWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
}

void UAuraWidgetController::BroadcastInitialValues()
{
}

void UAuraWidgetController::BindCallbacksToDependencies()
{
}

void UAuraWidgetController::BroadcastAbilityInfo()
{
	// 安全检查：确认能力系统已完成初始化
	if (!GetAuraASC()->bStartupAbilitiesGiven)return;
	// 创建单播委托实例（用于遍历每个能力）
	FForEachAbility BroadcastDelegate;
	// 使用Lambda绑定委托逻辑
	BroadcastDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
	{
		// 通过能力标签获取对应的UI信息
		FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AuraAbilitySystemComponent->GetAbilityTagFromSpec(AbilitySpec));
		// 添加输入标签信息
		Info.InputTag = AuraAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
		//从能力效果中获取能力状态
		Info.StatusTag = AuraAbilitySystemComponent->GetStatusFromSpec(AbilitySpec);
		// 广播更新UI的委托（触发所有绑定的UI元素）
		AbilityInfoDelegate.Broadcast(Info);

	});
	// 触发能力遍历操作
	GetAuraASC()->ForEachAbility(BroadcastDelegate);
}

/**
 * 获取Aura玩家控制器（带缓存优化）
 * 
 * 功能说明：
 * 1. 首次调用时执行类型转换并缓存结果
 * 2. 后续调用直接返回缓存值
 * 
 * @return 强类型玩家控制器指针
 */
AAuraPlayerController* UAuraWidgetController::GetAuraPC()
{
	// 惰性初始化：仅在首次访问时转换
	if(AuraPlayerController == nullptr)
	{
		// 安全转换：将基类PlayerController转为特定子类
		AuraPlayerController = Cast<AAuraPlayerController>(PlayerController);

	}
	return AuraPlayerController;  // 返回缓存结果
}

/**
 * 获取Aura玩家状态（带缓存优化）
 * 
 * 设计要点：
 * - 避免重复的类型转换开销
 * - 确保在整个生命周期内使用同一实例
 * 
 * @return 强类型玩家状态指针
 */
AAuraPlayerState* UAuraWidgetController::GetAuraPS()
{
	// 检查是否已缓存
	if(AuraPlayerState == nullptr)
	{
		// 执行安全类型转换
		AuraPlayerState = Cast<AAuraPlayerState>(PlayerState);
	}
	return AuraPlayerState;
}

/**
 * 获取Aura能力系统组件（带缓存优化）
 * 
 * 典型应用场景：
 * - 绑定属性变更委托
 * - 发送游戏事件
 * 
 * @return 强类型能力系统组件指针
 */
UAuraAbilitySystemComponent* UAuraWidgetController::GetAuraASC()
{
	if(AuraAbilitySystemComponent == nullptr)
	{
		// 转换基类ASC到项目特定子类
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent);

	}
	return AuraAbilitySystemComponent;
}

/**
 * 获取Aura属性集（带缓存优化）
 * 
 * 关键作用：
 * - 访问角色属性数据（生命值/法力值等）
 * - 监听属性变更事件
 *  
 * @return 强类型属性集指针
 */
UAuraAttributeSet* UAuraWidgetController::GetAuraAS()
{
	if(AuraAttributeSet == nullptr)
	{
		// 转换基类属性集到项目特定子类
		AuraAttributeSet = Cast<UAuraAttributeSet>(AttributeSet);
	}
	return AuraAttributeSet;
}