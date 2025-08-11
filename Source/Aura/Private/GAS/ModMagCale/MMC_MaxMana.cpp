// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ModMagCale/MMC_MaxMana.h"

#include "GAS/AuraAttributeSet.h"
#include "Interation/CombatInterface.h"

UMMC_MaxMana::UMMC_MaxMana()
{
	IntelligenceDef.AttributeToCapture =UAuraAttributeSet::GetIntelligenceAttribute();
	IntelligenceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	IntelligenceDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(IntelligenceDef);
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
	//默認玩家等級為1
	int32 PlayerLevel = 1;
	//檢查源對象是否實現了CombatInterface接口
	if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
	{
		//如果實現則獲取源對象等級
		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}
	// 根据体力和玩家等级计算最大生命值并返回
	return 50.f + 2.5f * Intelligence + 15.f * PlayerLevel;
}
