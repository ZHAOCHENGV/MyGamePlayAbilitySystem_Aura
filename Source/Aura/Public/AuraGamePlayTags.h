

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * 单列模式 ：它确保一个类只有一个实例，并提供一个全局访问点来获取该实例
 * 包含原生玩法标签的单例
 */
//结构体FAuraGamePlayTags, 用来存储与游戏标签相关数据
struct FAuraGamePlayTags
{
public:
	
	// 获取实例的静态方法。提供一个全局唯一的游戏标签实例，确保标签数据的统一性和一致性。
	static const FAuraGamePlayTags & Get(){return GamePlayTags;};
	// 初始化本地游戏标签
	static void InitializeNativeGameplayTags();

	/*
	 *	主要属性游戏标签
	 */
	//力量
	FGameplayTag Attributes_Primary_Strength;
	//智力
	FGameplayTag Attributes_Primary_Intelligence;
	//体力
	FGameplayTag Attributes_Primary_Resilience;
	//活力
	FGameplayTag Attributes_Primary_Vigor;


	
	/*
	 *	次要属性游戏标签
	 */

	//护甲
	FGameplayTag Attributes_Secondary_Armor;
	//护甲穿透
	FGameplayTag Attributes_Secondary_ArmorPenetration;
	//阻挡几率
	FGameplayTag Attributes_Secondary_BlockChance;
	//暴击率
	FGameplayTag Attributes_Secondary_CriticalHitChance;
	//暴击伤害
	FGameplayTag Attributes_Secondary_CriticalHitDamage;
	//暴击抗性
	FGameplayTag Attributes_Secondary_CriticalResistance;
	//血量恢复
	FGameplayTag Attributes_Secondary_HealthRegeneration;
	//魔法值恢复
	FGameplayTag Attributes_Secondary_ManaRegeneration;
	//最大生命
	FGameplayTag Attributes_Secondary_MaxHealth;
	//最大魔法值
	FGameplayTag Attributes_Secondary_MaxMana;

	
protected:
private:
	// 定义静态变量用于存储游戏标签
	// static 意味着该方法是静态的，可以直接通过类调用，不需要实例化对象。
	static FAuraGamePlayTags GamePlayTags;
};