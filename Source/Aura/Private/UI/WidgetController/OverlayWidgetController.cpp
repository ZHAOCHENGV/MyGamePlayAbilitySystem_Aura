// Fill out your copyright notice in the Description page of Project Settings.



#include "UI/WidgetController/OverlayWidgetController.h"

#include "GAS/AuraAttributeSet.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	UAuraAttributeSet * AuraAttributeSet = Cast<UAuraAttributeSet>(AttributeSet);
	//广播初始值
	OnHealthChanged.Broadcast(AuraAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(AuraAttributeSet->GetMaxHealth());
	OnManaChanged.Broadcast(AuraAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(AuraAttributeSet->GetMaxMana());
	

}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	//获取属性集，确保 AttributeSet 被正确转换为 UAuraAttributeSet 类型
	UAuraAttributeSet * AuraAttributeSet = Cast<UAuraAttributeSet>(AttributeSet);

	/// 绑定属性值变化的回调函数
	// GetGameplayAttributeValueChangeDelegate 是静态多播委托（Multicast Delegate）。
	// 当指定的 GameplayAttribute 值发生变化时，会触发该委托，通知所有绑定的回调函数。
	// 
	// 使用 AddUObject 方法将成员函数绑定到委托。
	// AddUObject 适用于单一绑定场景，且能安全地处理 UObject 的生命周期。
	// 如果需要支持蓝图或序列化，动态多播委托（AddDynamic）会更合适，但性能略低。
	// 绑定 Health 属性变化的回调
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetHealthAttribute())
	.AddUObject(this,&UOverlayWidgetController::HealthChanged);

	// 绑定 MaxHealth 属性变化的回调
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxHealthAttribute())
	.AddUObject(this,&UOverlayWidgetController::MaxHealthChanged);

	// 绑定 Mana 属性变化的回调
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetManaAttribute())
	.AddUObject(this,&UOverlayWidgetController::ManaChanged);

	// 绑定 MaxMana 属性变化的回调
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxManaAttribute())
	.AddUObject(this,&UOverlayWidgetController::MaxManaChanged);
}


void UOverlayWidgetController::HealthChanged(const FOnAttributeChangeData& Data) const
{
	//健康值属性变化时，广播数据的新值
	OnHealthChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data) const
{
	//最大健康值属性变化时，广播数据的新值
	OnMaxHealthChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::ManaChanged(const FOnAttributeChangeData& Data) const
{
	//魔法值属性变化时，广播数据的新值
	OnManaChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxManaChanged(const FOnAttributeChangeData& Data) const
{
	//最大魔法值属性变化时，广播数据的新值
	OnMaxManaChanged.Broadcast(Data.NewValue);
}
