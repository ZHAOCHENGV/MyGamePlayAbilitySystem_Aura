// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ModMagCale/MMC_MaxMana.h"

#include "GAS/AuraAttributeSet.h"
#include "Interation/CombatInterface.h"

UMMC_MaxMana::UMMC_MaxMana()
{
	IntelligenceDef.AttributeToCapture =UAuraAttributeSet::GetIntelligenceAttribute();
	IntelligenceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	IntelligenceDef.bSnapshot = false;
}

float UMMC_MaxMana::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// 从源和目标对象收集标签
	const FGameplayTagContainer * SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer * TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	// 创建评估参数对象，并传入源和目标标签
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;
	// 定义一个变量来存储捕获的值
	float Intelligence = 0.f;
	GetCapturedAttributeMagnitude(IntelligenceDef,Spec,EvaluateParameters,Intelligence);
	Intelligence = FMath::Max<float>(Intelligence,0.f);
	// 获取施法者（源对象）的 CombatInterface 接口
	ICombatInterface * CombatInterface = Cast<ICombatInterface>(Spec.GetContext().GetSourceObject());
	// 获取玩家等级
	const int32 PlayerLevel = CombatInterface->GetPlayerLevel();
	// 根据体力和玩家等级计算最大生命值并返回
	return 30.f + 2.5f * Intelligence + 5.f * PlayerLevel;
}
