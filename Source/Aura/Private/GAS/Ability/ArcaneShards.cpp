// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/ArcaneShards.h"

FString UArcaneShards::GetDescription(int32 Level)
{
	const int32 ScaleDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if(Level == 1)
	{
		return FString::Printf(TEXT(
			"<Title>奥术碎片</>\n\n"
			"<Small>等级: %d</>\n"
			"<Small>消耗: </><ManaCost>%.1f</><Small> 法力值</>\n"
			"<Small>冷却: </><Cooldown>%.1f</><Small> 秒</>\n\n"
			"<Default>召唤一个奥术碎片，并照成:</>"
			"<Damage>%d</>"
			"<Default>点根据范围衰减的奥术伤害，并照成击飞效果</>"),
			Level,
			ManaCost,
			Cooldown,
			ScaleDamage);
	}
	else {
		return FString::Printf(TEXT(
			"<Title>奥术碎片</>\n\n"
			"<Small>等级: %d</>\n"
			"<Small>消耗: </><ManaCost>%.1f</><Small> 法力值</>\n"
			"<Small>冷却: </><Cooldown>%.1f</><Small> 秒</>\n\n"
			"<Default>在范围内召唤%d个奥术碎片，并照成:</>"
			"<Damage>%d</>"
			"<Default>点根据范围衰减的奥术伤害，并照成击飞效果</>"),
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level,MaxNumShards),
			ScaleDamage);
	}
}

FString UArcaneShards::GetNextLevelDescription(int32 Level)
{
	const int32 ScaleDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			"<Title>下一等级奥术碎片</>\n\n"
			"<Small>等级: %d</>\n"
			"<Small>消耗: </><ManaCost>%.1f</><Small> 法力值</>\n"
			"<Small>冷却: </><Cooldown>%.1f</><Small> 秒</>\n\n"
			"<Default>在范围内召唤%d个奥术碎片，并照成:</>"
			"<Damage>%d</>"
			"<Default>点范围衰减的奥术伤害，并照成击飞效果</>"),
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level,MaxNumShards),
			ScaleDamage);
}
