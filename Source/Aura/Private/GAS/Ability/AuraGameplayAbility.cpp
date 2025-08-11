// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/AuraGameplayAbility.h"

#include "GAS/AuraAttributeSet.h"

FString UAuraGameplayAbility::GetDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>%s,</><Level>Level:%d</>"),L"默认技能名称 - 某某某某某某某某某某某某某某某某某某某某某某某某某某某某某某",Level);
}

FString UAuraGameplayAbility::GetNextLevelDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>下一等级：</><Level>%d</> \n <Default>伤害提高，魔力值消耗减小.</>"),Level);
}

FString UAuraGameplayAbility::GetLockedDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>等级到达 %d 解锁技能</>"),Level);
}


float UAuraGameplayAbility::GetManaCost(float InLevel) const
{
	float ManaCost = 0.0f;
	if (const UGameplayEffect* CostEffect = GetCostGameplayEffect())
	{
		for (FGameplayModifierInfo Mod : CostEffect->Modifiers)
		{
			if (Mod.Attribute == UAuraAttributeSet::GetManaAttribute())
			{
				Mod.ModifierMagnitude.GetStaticMagnitudeIfPossible(InLevel, ManaCost);
				break; 
			}
		}
	}
	return ManaCost;
}


float UAuraGameplayAbility::GetCooldown(float InLevel) const
{
	float Cooldown = 0.0f;
	if (const UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect())
	{
		CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(InLevel, Cooldown);
	}
	return Cooldown;
}
