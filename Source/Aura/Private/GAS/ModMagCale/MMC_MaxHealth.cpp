// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ModMagCale/MMC_MaxHealth.h"

#include "GAS/AuraAttributeSet.h"
#include "Interation/CombatInterface.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	// 设置捕获属性：体力（Vigor）属性
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
	// 设置属性的捕获来源为“目标角色”（Target）
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	// 设置是否捕获快照值为 false，即捕获实时值（而非快照）
	VigorDef.bSnapshot = false;
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// 从源和目标对象收集标签
	const FGameplayTagContainer * SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer * TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	// 创建评估参数对象，并传入源和目标标签
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;
	// 定义一个变量来存储捕获的体力（Vigor）值
	float Vigor = 0.f;
	// 根据传入的捕获定义VigorDef,使用 GetCapturedAttributeMagnitude 用来从效果规范Spec中捕获体力值Vigor
	GetCapturedAttributeMagnitude(VigorDef,Spec,EvaluateParameters,Vigor);
	// 确保体力值不为负数
	Vigor = FMath::Max<float>(Vigor,0.F);
	// 获取施法者（源对象）的 CombatInterface 接口
	ICombatInterface * CombatInterface = Cast<ICombatInterface>(Spec.GetContext().GetSourceObject());
	// 获取玩家等级
	const int32 PlayerLevel = CombatInterface->GetPlayerLevel();
	// 根据体力和玩家等级计算最大生命值并返回
	return 80.f + 2.5f * Vigor + 10.f * PlayerLevel;
}
