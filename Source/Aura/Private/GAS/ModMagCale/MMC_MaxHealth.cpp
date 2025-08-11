// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ModMagCale/MMC_MaxHealth.h"

#include "GAS/AuraAttributeSet.h"
#include "Interation/CombatInterface.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	// 1. 定义要捕获的属性：体力（Vigor）
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
	// 2. 设置属性捕获来源：从目标角色（被施加效果的Actor）捕获
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	// 3. 关闭快照模式（实时捕获当前值）
	VigorDef.bSnapshot = false;
	// 4. 注册需要捕获的属性到系统
	RelevantAttributesToCapture.Add(VigorDef);
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
	//默認玩家等級為1
	int32 PlayerLevel = 1;
	//檢查源對象是否實現了CombatInterface接口
	if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
	{
		//如果實現則獲取源對象等級
		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}
	// 根据体力和玩家等级计算最大生命值并返回
	return 80.f + 2.5f * Vigor + 10.f * PlayerLevel;
}
