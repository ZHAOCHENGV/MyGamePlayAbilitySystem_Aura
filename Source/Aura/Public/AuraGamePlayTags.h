

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
	FGameplayTag Attributes_Secondary_CriticalHitResistance;
	//血量恢复
	FGameplayTag Attributes_Secondary_HealthRegeneration;
	//魔法值恢复
	FGameplayTag Attributes_Secondary_ManaRegeneration;
	//最大生命
	FGameplayTag Attributes_Secondary_MaxHealth;
	//最大魔法值
	FGameplayTag Attributes_Secondary_MaxMana;

	/*
	 * 伤害类型抗性
	 */
	FGameplayTag Attributes_Resistance_Fire;
	FGameplayTag Attributes_Resistance_Lightning;
	FGameplayTag Attributes_Resistance_Arcane;
	FGameplayTag Attributes_Resistance_Physical;

	/*
	 * 元属性
	 */
	FGameplayTag Attributes_Meta_IncomingXP;
	
	/*
	 *	输入操作标签
	 */
	//左键
	FGameplayTag InputTag_LMB;
	//右键
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;
	FGameplayTag InputTag_5;
	FGameplayTag InputTag_Passive_1;
	FGameplayTag InputTag_Passive_2;
	
	/*
	 * 伤害类型标签
	 */
	FGameplayTag Damage;
	//火焰伤害类型
	FGameplayTag Damage_Fire;
	//闪电伤害类型
	FGameplayTag Damage_Lightning;
	//奥术伤害类型
	FGameplayTag Damage_Arcane;
	//物理伤害类型
	FGameplayTag Damage_Physical;
	//抗性伤害类型
	TMap<FGameplayTag,FGameplayTag> DamageTypesToResistance;

	/*
	 * Debuff标签
	 */
	//燃烧
	FGameplayTag Debuff_Burn;
	//眩晕
	FGameplayTag Debuff_Stun;
	//法术
	FGameplayTag Debuff_Arcane;
	//物理
	FGameplayTag Debuff_Physical;
	//伤害类型减益数组
	TMap<FGameplayTag,FGameplayTag> DamageTypesToDebuffs;

	/*
	 * Debuff主要参数
	 */
	//减益几率
	FGameplayTag Debuff_Chance;
	//减益伤害
	FGameplayTag Debuff_Damage;
	//减益频率
	FGameplayTag Debuff_Frequency;
	//减益持续时间
	FGameplayTag Debuff_Duration;

	/*
	 * 技能冷却
	 */
	FGameplayTag Cooldown_Fire_FireBolt;
	FGameplayTag Cooldown_Lightning_Electrocute;
	FGameplayTag Cooldown_Arcane_ArcaneShards;

	/*
	 * 技能
	 */

	FGameplayTag Abilities_Fire_FireBolt;
	FGameplayTag Abilities_Lightning_Electrocute;
	FGameplayTag Abilities_Arcane_ArcaneShards;

	
	FGameplayTag Abilities_None;
	FGameplayTag Abilities_Attack;
	FGameplayTag Abilities_Summon;
	FGameplayTag Abilities_HitReact;

	/*
	 * 技能状态
	 */
	FGameplayTag Abilities_Status_Locked;
	FGameplayTag Abilities_Status_Eligible;
	FGameplayTag Abilities_Status_Unlocked;
	FGameplayTag Abilities_Status_Equipped;

	/*
	 * 技能进攻类型
	 */
	FGameplayTag Abilities_Type_Offensive;
	FGameplayTag Abilities_Type_Passive;
	FGameplayTag Abilities_Type_None;
	


	FGameplayTag Abilities_Passive_HaloOfProtection;
	FGameplayTag Abilities_Passive_LifeSiphon;
	FGameplayTag Abilities_Passive_ManaSiphon;

	/*
	 * 效果
	 */
	FGameplayTag Effects_HitReact;

	/*
	 * 战斗插槽
	 */
	FGameplayTag CombatSocket_Weapon;
	FGameplayTag CombatSocket_RightHand;
	FGameplayTag CombatSocket_LeftHand;
	FGameplayTag CombatSocket_Tail;

	/*
	* 蒙太奇插槽
	*/
	FGameplayTag Montage_Attack_1;
	FGameplayTag Montage_Attack_2;
	FGameplayTag Montage_Attack_3;
	FGameplayTag Montage_Attack_4;

	/*
	 * 玩家操作
	 */
	FGameplayTag Player_Block_InputPressed;
	FGameplayTag Player_Block_InputHeld;
	FGameplayTag Player_Block_InputReleased;
	FGameplayTag Player_Block_CursorTrace;
	
private:
	// 定义静态变量用于存储游戏标签
	// static 意味着该方法是静态的，可以直接通过类调用，不需要实例化对象。
	static FAuraGamePlayTags GamePlayTags;
};