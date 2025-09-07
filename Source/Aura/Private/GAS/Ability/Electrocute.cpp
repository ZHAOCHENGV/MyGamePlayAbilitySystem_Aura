// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/Electrocute.h"

FString UElectrocute::GetDescription(int32 Level)
{
	
	const int32 ScaleDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if(Level == 1)
	{
		return FString::Printf(TEXT(
			"<Title>闪电</>\n\n"
			"<Small>等级: %d</>\n"
			"<Small>消耗: </><ManaCost>%.1f</><Small> 法力值</>\n"
			"<Small>冷却: </><Cooldown>%.1f</><Small> 秒</>\n\n"
			"<Default>发射一道闪电，并照成:</>"
			"<Damage>%d</>"
			"<Default>点闪电伤害，有几率照成眩晕效果</>"),
			Level,
			ManaCost,
			Cooldown,
			ScaleDamage);
	}
	else {
		return FString::Printf(TEXT(
			"<Title>闪电</>\n\n"
			"<Small>等级: %d</>\n"
			"<Small>消耗: </><ManaCost>%.1f</><Small> 法力值</>\n"
			"<Small>冷却: </><Cooldown>%.1f</><Small> 秒</>\n\n"
			"<Default>发射%d道闪电，并照成:</>"
			"<Damage>%d</>"
			"<Default>点闪电伤害，有几率照成眩晕效果</>"),
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level,MaxNumShockTargets),
			ScaleDamage);
	}
}


FString UElectrocute::GetNextLevelDescription(int32 Level)
{
	const int32 ScaleDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			"<Title>下一等级闪电</>\n\n"
			"<Small>等级: %d</>\n"
			"<Small>消耗: </><ManaCost>%.1f</><Small> 法力值</>\n"
			"<Small>冷却: </><Cooldown>%.1f</><Small> 秒</>\n\n"
			"<Default>发射%d道闪电，并照成:</>"
			"<Damage>%d</>"
			"<Default>点闪电伤害，有几率照成眩晕效果</>"),
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level,MaxNumShockTargets),
			ScaleDamage);
}
