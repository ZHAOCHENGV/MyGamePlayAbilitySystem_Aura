// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/AttributeMenuWidgetController.h"

#include "AuraGamePlayTags.h"
#include "GAS/AuraAbilitySystemComponent.h"
#include "GAS/AuraAttributeSet.h"
#include "GAS/Data/AttributeInfo.h"
#include "Player/AuraPlayerState.h"

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	// 确保 AttributeSet 是有效的，并且能成功转换为 UAuraAttributeSet 类型
	UAuraAttributeSet * AS = CastChecked<UAuraAttributeSet>(AttributeSet);

	for (auto & Pair : AS->TagsToAttributes)
	{
		//const FOnAttributeChangeData & Data: 属性值变化时传入的回调数据，包含变化后的新值和变化前的旧值。
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this,Pair](const FOnAttributeChangeData & Data)
			{
				BroadcastAttributeInfo(Pair.Key, Pair.Value());
			}

		);
	}

	
		// 为属性点变更委托添加Lambda回调
		GetAuraPS()->OnAttributePointsChangedDelegate.AddLambda(
		[this](int32 Points)
		{
			// 将玩家状态的属性点变更事件转发到本地委托
			AttributePointsChangedDelegate.Broadcast(Points);
		}
		
	);


}

void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	// 确保 AttributeSet 是有效的，并且能成功转换为 UAuraAttributeSet 类型
	UAuraAttributeSet * AS = CastChecked<UAuraAttributeSet>(AttributeSet);
	// 确保 AttributeInfo 不是空指针
	check(AttributeInfo);
	// 遍历属性系统(AS)中的所有标签-属性映射
	for (auto & Pair : AS->TagsToAttributes)
	{
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}

	AttributePointsChangedDelegate.Broadcast(GetAuraPS()->GetAttributePoints());
}

void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	AuraASC->UpgradeAttribute(AttributeTag);
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag,const FGameplayAttribute& Attribute)const
{
	// 根据标签查找属性信息
	FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	// 执行委托获取属性值并存储到Info中
	//Pair.Value 是一个 FAttributeSignature 类型的委托
	//GetNumericValue(AS)是FGameplayAttribute 的 GetNumericValue(UAttributeSet*) 方法
	//参数 AS 是一个 UAttributeSet 的指针，表示属性的实际存储位置。
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	// 广播属性信息
	AttributeInfoDelegate.Broadcast(Info);
}
