// Fill out your copyright notice in the Description page of Project Settings.



#include "UI/WidgetController/OverlayWidgetController.h"

#include "AuraGamePlayTags.h"
#include "GAS/AuraAbilitySystemComponent.h"
#include "GAS/AuraAttributeSet.h"
#include "GAS/Data/AbilityInfo.h"
#include "GAS/Data/LevelUpInfo.h"
#include "Player/AuraPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValues()
{

	//广播初始值
	OnHealthChanged.Broadcast(GetAuraAS()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetAuraAS()->GetMaxHealth());
	OnManaChanged.Broadcast(GetAuraAS()->GetMana());
	OnMaxManaChanged.Broadcast(GetAuraAS()->GetMaxMana());
	

}

void UOverlayWidgetController::BindCallbacksToDependencies()
{

	
	GetAuraPS()->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);
	GetAuraPS()->OnLevelChangedDelegate.AddLambda(
	[this](int32 NewLevel)
	{
		OnPlayerLevelChangedDelegate.Broadcast(NewLevel);
	}
	);
	

	/// 绑定属性值变化的回调函数
	// GetGameplayAttributeValueChangeDelegate 是静态多播委托（Multicast Delegate）。
	// 当指定的 GameplayAttribute 值发生变化时，会触发该委托，通知所有绑定的回调函数。
	// 使用 AddUObject 方法将成员函数绑定到委托。
	// AddUObject 适用于单一绑定场景，且能安全地处理 UObject 的生命周期。
	// 如果需要支持蓝图或序列化，动态多播委托（AddDynamic）会更合适，但性能略低。
	// 绑定 Health 属性变化的回调
	//使用Lambda匿名函数，广播委托
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAS()->GetHealthAttribute())
	.AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			//健康值属性变化时，广播数据的新值
			OnHealthChanged.Broadcast(Data.NewValue);
		}
	);

	// 绑定 MaxHealth 属性变化的回调
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAS()->GetMaxHealthAttribute())
	.AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			//属性变化时，广播数据的新值
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		}
	);

	// 绑定 Mana 属性变化的回调
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAS()->GetManaAttribute())
	.AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			//属性变化时，广播数据的新值
			OnManaChanged.Broadcast(Data.NewValue);
		}
	);

	// 绑定 MaxMana 属性变化的回调
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAS()->GetMaxManaAttribute())
	.AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			//性变化时，广播数据的新值
			OnMaxManaChanged.Broadcast(Data.NewValue);
		}
	);

	// 将AbilitySystemComponent转换为Aura专用版本
	if (GetAuraASC())
	{
		//綁定裝備武器委託
		GetAuraASC()->AbilityEquipped.AddUObject(this,&UOverlayWidgetController::OnAbilityEquipped);
		// 检查是否已经完成初始能力赋予
		if (GetAuraASC()->bStartupAbilitiesGiven)
		{
			// 如果已完成，直接执行初始化逻辑
			BroadcastAbilityInfo();
		}
		else
		{
			// 如果未完成，将回调函数绑定到委托
			// 使用AddUObject确保安全绑定UObject成员函数
			// 当AbilitiesGivenDelegate被广播时，会自动调用OnInitializeStartupAbilities
			GetAuraASC()->AbilitiesGivenDelegate.AddUObject(
				this,// 持有回调方法的对象
				&UOverlayWidgetController::BroadcastAbilityInfo// 回调方法
				);
		}
		
			// EffectAbilityTags自定义类型中定义的一个委托，通常用于广播游戏效果相关事件。
			// AddLambda 方法将一个 Lambda 表达式（匿名函数）绑定到该委托上，使得每次事件触发时都会执行该 Lambda 表达式。
			// Lambda 表达式  [捕获列表](参数列表) -> 返回类型 { 函数体 };
			// 捕获列表 []：决定 Lambda 表达式可以访问的外部变量或成员函数。如果外部变量在表达式中有引用或拷贝需求，需要在此显式声明，内部的成员函数，填写this即可。
			// 参数列表 ()：和普通函数一样，可以接受参数，支持类型定义和默认值。
			// 返回类型 ->（可选）：如果返回类型能自动推导，可以省略；否则需要显式指定。
			// 函数体 {}：定义 Lambda 表达式的核心逻辑。
			GetAuraASC()->EffectAbilityTags.AddLambda(
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
	
	
}


/**
 * 处理经验值变化的UI更新逻辑
 * @param NewXP 新的经验值总量
 * 
 * 功能说明：
 * 1. 计算当前经验对应的等级
 * 2. 计算当前等级的经验进度百分比
 * 3. 通过委托通知UI更新
 * 
 * 注意：需确保LevelUpInfo数据资产已正确配置
 */
void UOverlayWidgetController::OnXPChanged(int32 NewXP)
{
	// 获取等级配置数据资产
	const ULevelUpInfo* LevelUpInfo = GetAuraPS()->LevelUpInfo;
	// 重要安全检查：确保数据资产存在
	// checkf在开发版本会触发断言并显示错误信息
	checkf(LevelUpInfo, TEXT("无法找到LevelUpInfo,请填写AuraPlayerState"));

	// 计算当前经验对应的等级
	const int32 Level = LevelUpInfo->FindLevelForXp(NewXP);
	// 获取配置表中的最大等级
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();

	// 验证等级有效性
	if(Level <=MaxLevel && Level >0)
	{
		// 获取当前等级升级所需总经验
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		// 获取上一级的总经验要求
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;
		// 计算当前等级区间所需经验差值
		const int32 DeltaLevelRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		// 计算在当前等级区间内的经验值
		const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;
		// 计算经验条百分比（浮点除法）
		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelRequirement);
		// 广播经验百分比变更（触发UI更新）
		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);

		
	} 
	
}

/**
 * 处理技能装备事件（用于覆盖层UI）
 * 
 * @param AbilityTag   当前装备的技能标签
 * @param Status       新的技能状态（如已装备、已解锁等）
 * @param Slot         当前装备的技能槽位标签
 * @param PreviousSlot 上一次装备的技能槽位标签
 * 
 * 功能说明：
 * 1. 清除（重置）上一个技能槽位的技能显示。
 * 2. 更新并广播新装备技能的状态到界面。
 * 3. 通过委托通知所有监听者，驱动UI等逻辑响应技能变更。
 * 
 * 虚幻机制：
 * - 广播使用了`AbilityInfoDelegate`多播委托，典型的事件驱动UI刷新方式。
 * - 标签系统依赖于GameplayTags单例，便于全局一致性管理技能与状态。
 */
void UOverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	// 1. 获取标签单例，便于后续引用常量标签
	const FAuraGamePlayTags& GameplayTags = FAuraGamePlayTags::Get();

	// 2. 组装上一个槽位的技能信息，表示该槽已被清空
	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked; // 状态重置为未装备
	LastSlotInfo.InputTag  = PreviousSlot;                           // 设置为上一个槽位
	LastSlotInfo.AbilityTag= GameplayTags.Abilities_None;            // 技能标签清空

	// 3. 广播先前槽位被清空的消息，通知UI等刷新
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	// 4. 查找当前装备技能的信息，并更新新状态与槽位
	FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status; // 赋值新状态
	Info.InputTag  = Slot;   // 赋值新槽位

	// 5. 广播新装备技能的信息，驱动UI等响应
	AbilityInfoDelegate.Broadcast(Info);
}


