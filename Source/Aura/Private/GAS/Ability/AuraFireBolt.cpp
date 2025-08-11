// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/AuraFireBolt.h"
#include "AuraGamePlayTags.h"

/**
 * 获取指定等级下火焰箭技能的描述文本
 *
 * @param Level   技能等级
 * @return        格式化后的技能描述字符串
 *
 * 功能说明：
 * 根据传入的技能等级，动态生成火焰箭技能在该等级下的详细描述文本。
 * 描述中包含技能名称、发射火球数量、造成的火焰伤害等信息，支持富文本标签，便于UI高亮显示关键数据。
 *
 * 详细说明：
 * - 首先通过伤害类型映射表，查找火焰伤害在当前等级下的数值（见下方Damage变量详细解释）。
 * - 若为1级，技能名称为“1级火焰箭”，描述中写明“发射一道火焰”，
 *   并展示该等级火焰伤害数值和燃烧概率说明。
 * - 若为2级及以上，技能名称为“X级火焰箭”，描述中写明“发射N道火焰”；
 *   其中N为当前等级和最大弹道数（NumProjectiles）中的较小值，用于限制高等级下的投射物数量不会超过技能设定上限。
 * - 返回拼接好的富文本字符串，便于在UI中高亮显示技能核心效果。
 */
FString UAuraFireBolt::GetDescription(int32 Level)
{
	/*
		* - DamageTypes：这是一个 TMap<FGameplayTag, FAuraDamageInfo> 类型的成员变量，存储了不同伤害类型（如火焰、冰霜等）与其对应的数值成长信息。
		* - FAuraGamePlayTags::Get().Damage_Fire：静态单例，返回全局定义的火焰伤害标签，用于查找火焰类型的伤害信息。
		* - DamageTypes[...]: 通过火焰伤害标签在映射表中查找火焰类型的成长数据FAuraDamageInfo。
		* - GetValueAtLevel(Level)：这是FAuraDamageInfo的成员函数，根据传入的技能等级Level，返回该等级下的具体伤害数值（通常查表或插值实现）。
		* - 所以整句代码的作用是：根据当前技能等级，从技能的伤害成长表中查出对应的火焰伤害数值，用于技能描述或实际计算。
	 */
	const int32 ScaleDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if(Level == 1)
	{
		return FString::Printf(TEXT(
			"<Title>火焰箭</>\n\n"
			"<Small>等级: %d</>\n"
			"<Small>消耗: </><ManaCost>%.1f</><Small> 法力值</>\n"
			"<Small>冷却: </><Cooldown>%.1f</><Small> 秒</>\n\n"
			"<Default>发射一道火焰，撞击时会爆炸并照成:</>"
			"<Damage>%d</>"
			"<Default>点火焰伤害，有几率照成燃烧效果</>"),
			Level,
			ManaCost,
			Cooldown,
			ScaleDamage);
	}
		return FString::Printf(TEXT(
			"<Title>火焰箭</>\n\n"
			"<Small>等级: %d</>\n"
			"<Small>消耗: </><ManaCost>%.1f</><Small> 法力值</>\n"
			"<Small>冷却: </><Cooldown>%.1f</><Small> 秒</>\n\n"
			"<Default>发射%d道火焰，撞击时会爆炸并照成:</>"
			"<Damage>%d</>"
			"<Default>点火焰伤害，有几率照成燃烧效果</>"),
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level,NumProjectiles),
			ScaleDamage);
}

/**
 * 获取下一等级火焰箭技能的描述文本
 *
 * @param Level   技能下一等级
 * @return        下一等级的技能描述字符串
 *
 * 功能说明：
 * 动态生成下一等级火焰箭技能的描述文本，让玩家预览升级后的技能效果。
 * 与当前等级描述类似，但参数为下一等级。
 *
 * 详细说明：
 * - 计算下一等级下的火焰伤害数值。
 * - 描述中写明升级后可发射的火焰弹数量（受最大弹道上限NumProjectiles限制）。
 * - 主要用于技能升级界面，帮助玩家决策是否消耗点数进行升级。
 */
FString UAuraFireBolt::GetNextLevelDescription(int32 Level)
{
	const int32 ScaleDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			"<Title>下一等级火焰箭</>\n\n"
			"<Small>等级: %d</>\n"
			"<Small>消耗: </><ManaCost>%.1f</><Small> 法力值</>\n"
			"<Small>冷却: </><Cooldown>%.1f</><Small> 秒</>\n\n"
			"<Default>发射%d道火焰，撞击时会爆炸并照成:</>"
			"<Damage>%d</>"
			"<Default>点火焰伤害，有几率照成燃烧效果</>"),
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level,NumProjectiles),
			ScaleDamage);
}
