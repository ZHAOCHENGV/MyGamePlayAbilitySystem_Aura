// Fill out your copyright notice in the Description page of Project Settings.



#include "UI/WidgetController/OverlayWidgetController.h"

#include "GAS/AuraAbilitySystemComponent.h"
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

	// EffectAbilityTags自定义类型中定义的一个委托，通常用于广播游戏效果相关事件。
	// AddLambda 方法将一个 Lambda 表达式（匿名函数）绑定到该委托上，使得每次事件触发时都会执行该 Lambda 表达式。
	// Lambda 表达式  [捕获列表](参数列表) -> 返回类型 { 函数体 };
	// 捕获列表 []：决定 Lambda 表达式可以访问的外部变量或成员函数。如果外部变量在表达式中有引用或拷贝需求，需要在此显式声明，内部的成员函数，填写this即可。
	// 参数列表 ()：和普通函数一样，可以接受参数，支持类型定义和默认值。
	// 返回类型 ->（可选）：如果返回类型能自动推导，可以省略；否则需要显式指定。
	// 函数体 {}：定义 Lambda 表达式的核心逻辑。
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->EffectAbilityTags.AddLambda(
			// 接收一个 FGameplayTagContainer 类型的引用，包含一组游戏标签。
			[this](const FGameplayTagContainer & AssetTags)
		{
			// 循环遍历容器中的标签
			for (const FGameplayTag& Tag : AssetTags)
			{
				// 创建一个 FGameplayTag 类型的标签并初始化为名为 "Message" 的标签。
				FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
				// 检查当前 Tag 是否与 MessageTag 匹配。
				// - MatchesTag：比较两个标签是否相同，或 Tag 是否是 MessageTag 的父级或子级。
				// - 返回值：true 表示匹配，false 表示不匹配。
				if (Tag.MatchesTag(MessageTag))
				{
					
					//打印标签
					GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Red, FString(Tag.ToString()));
					// 根据标签从 MessageWidgetDataTable 数据表中获取对应的行数据（类型为 FUIWidgetRow）。
					// - Tag：要查找的标签，表示行的标识。
					// 返回值：找到的行数据指针，若未找到则返回 nullptr。
					const FUIWidgetRow* Row =GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable,Tag);
					// 如果找到对应的行数据，则通过动态多播委托广播该行的数据。
					// - MessageWidgetRowSignature：动态多播委托，用于通知其他系统或蓝图。
					MessageWidgetRowSignature.Broadcast(*Row);
				}
				
			}

		}

	);
	
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
