// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ExecCalc/ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "AuraGamePlayTags.h"
#include "GAS/AuraAbilitySystemLibrary.h"
#include "GAS/AuraAttributeSet.h"
#include "GAS/Data/CharacterClassInfo.h"
#include "Interation/CombatInterface.h"


/**
 * @brief 伤害执行用的“属性捕获定义集合”（Execution：在计算伤害时从源/目标读取所需属性）
 *
 * 功能说明：
 * - DECLARE_ATTRIBUTE_CAPTUREDEF(Name)：声明一个捕获定义句柄（如 ArmorDef），供执行期按定义读取属性值；
 * - DEFINE_ATTRIBUTE_CAPTUREDEF(AttrSet, Name, From, bSnapshot)：
 *   指定属性所在的 AttributeSet、属性名、捕获来源（Source/Target）以及是否快照（bSnapshot）。
 *
 * 术语速记：
 * - Source/Target：来源/目标 ASC（施法者/受击者）；bSnapshot=true 表示在GE创建时快照，false表示在执行时实时读取。
 */
struct AuraDamageStatics
{
	// 声明“护甲”捕获定义
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	// 声明“护甲穿透”捕获定义（注意：项目中命名为 ArmorPenetrion）
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetrion);
	// 声明“格挡几率”捕获定义
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	// 声明“暴击率”捕获定义
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	// 声明“暴击伤害”捕获定义
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);
	// 声明“暴击抗性”捕获定义
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance);
	// 声明“火焰抗性”捕获定义
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance);
	// 声明“闪电抗性”捕获定义
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance);
	// 声明“奥术抗性”捕获定义
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArcaneResistance);
	// 声明“物理抗性”捕获定义
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance);

	// 构造时完成具体捕获规则的定义（指定属性集/来源端/是否快照）
	AuraDamageStatics()
	{
		// 目标端读取“护甲”，执行时读取（非快照）
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);

		// 目标端读取“格挡几率”，执行时读取（非快照）
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);

		// 来源端读取“护甲穿透”，执行时读取（非快照）
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetrion, Source, false);

		// 来源端读取“暴击率”，执行时读取（非快照）
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, false);

		// 来源端读取“暴击伤害”，执行时读取（非快照）
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitDamage, Source, false);

		// 目标端读取“暴击抗性”，执行时读取（非快照）
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResistance, Target, false);

		// 目标端读取“火焰抗性”，执行时读取（非快照）
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, FireResistance, Target, false);

		// 目标端读取“闪电抗性”，执行时读取（非快照）
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, LightningResistance, Target, false);

		// 目标端读取“奥术抗性”，执行时读取（非快照）
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArcaneResistance, Target, false);

		// 目标端读取“物理抗性”，执行时读取（非快照）
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, PhysicalResistance, Target, false);
	}
};


static const AuraDamageStatics& DamageStatics()
{
	// 静态实例，用于提供全局访问。
	static AuraDamageStatics DStatics;
	return DStatics;
}


UExecCalc_Damage::UExecCalc_Damage()
{
	//将 Armor 属性的捕获规则添加到当前计算所需的属性列表中
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrionDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);

	RelevantAttributesToCapture.Add(DamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightningResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArcaneResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalResistanceDef);
	
}


/**
 * @brief 自定义伤害执行：根据伤害类型与目标抗性判定减益触发概率
 *
 * @param ExecutionParams 执行期上下文（源/目标 ASC、已捕获属性、Spec 等）
 * @param Spec            本次执行所属的 GE 规格（包含 SetByCaller、捕获快照、标签聚合等）
 * @param EvaluateParameters 捕获求值所需的标签上下文（源/目标聚合标签）
 * @param InTagsToDefs    标签到属性捕获定义的映射（用于按标签读取已捕获的属性）
 *
 * 功能说明：
 * - 针对每种伤害类型：读取该类型的 SetByCaller 伤害值，如果存在则按“来源减益几率”与“目标对应抗性”计算有效减益概率；
 * - 随机判定是否触发对应的 Debuff（如灼烧/感电/流血等）；触发时应附加后续处理（此处预留 if 分支）。
 *
 * 注意事项：
 * - 读取 SetByCaller 时使用默认值 -1 表示“未设置”，通过阈值判断过滤；
 * - 目标抗性捕获值建议下限为 0，上限可按需要 Clamp 到 100；
 * - 触发后应在 if(bDebuff) 内附加 GE/标签或写入输出（当前留空待接入具体 Debuff 逻辑）。
 */
void UExecCalc_Damage::DetermineDebuff(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                       const FGameplayEffectSpec& Spec,
                                       FAggregatorEvaluateParameters EvaluateParameters,
                                       const TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition>& InTagsToDefs) const
{
	// 获取全局标签（包含 伤害类型->Debuff、伤害类型->抗性 映射）
	const FAuraGamePlayTags& GamePlayTags = FAuraGamePlayTags::Get();

	// 遍历“伤害类型 -> Debuff类型”的映射，逐一判定是否触发 Debuff
	for (TTuple<FGameplayTag, FGameplayTag> Pair : GamePlayTags.DamageTypesToDebuffs)
	{
		// 当前伤害类型与其对应的 Debuff 类型
		const FGameplayTag& DamageType = Pair.Key;
		const FGameplayTag& DebuffType = Pair.Value;

		// 读取该伤害类型的 SetByCaller 数值（未设置时返回 -1）
		const float TypeDamage = Spec.GetSetByCallerMagnitude(DamageType, false, -1.f);

		// 只有当该类型伤害存在（> -0.5 视为有效）时才参与 Debuff 判定
		if (TypeDamage > -0.5f)
		{
			// 来源配置的基础 Debuff 触发概率（百分比，来自 SetByCaller）
			const float SourceDebuffChance = Spec.GetSetByCallerMagnitude(GamePlayTags.Debuff_Chance, false, -1.f);

			// 目标对应的“该伤害类型的抗性”标签（例如 火伤->火抗）
			float TargetDebuffResistance = 0.f;
			const FGameplayTag& ResistanceTag = GamePlayTags.DamageTypesToResistance[DamageType];

			// 通过捕获定义计算目标抗性数值（依赖 InTagsToDefs 的映射）
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(InTagsToDefs[ResistanceTag], EvaluateParameters, TargetDebuffResistance);

			// 抗性不低于 0（可按需要再 Clamp 到 100）
			TargetDebuffResistance = FMath::Max<float>(TargetDebuffResistance, 0.f);

			// 有效触发概率 = 基础概率 * (100 - 抗性) / 100
			const float EffectiveDebuffChance = SourceDebuffChance * (100.f - TargetDebuffResistance) / 100.f;

			// 随机判定是否触发（百分制，含端点时可用 <=）
			const bool bDebuff = FMath::RandRange(1, 100) < EffectiveDebuffChance;

			// 触发后：在此附加 Debuff（如应用 DoT GE、添加状态标签等）——留给具体项目逻辑接入
			if (bDebuff)
			{
				FGameplayEffectContextHandle ContextHandle = Spec.GetContext();
				UAuraAbilitySystemLibrary::SetIsSuccessfulDebuff(ContextHandle, true);
				UAuraAbilitySystemLibrary::SetDamageType(ContextHandle, DamageType);

				const float DebuffDamage = Spec.GetSetByCallerMagnitude(GamePlayTags.Debuff_Damage, false, -1.f);
				const float DebuffDuration = Spec.GetSetByCallerMagnitude(GamePlayTags.Debuff_Duration, false, -1.f);
				const float DebuffFrequency = Spec.GetSetByCallerMagnitude(GamePlayTags.Debuff_Frequency, false, -1.f);

				UAuraAbilitySystemLibrary::SetDebuffDamage(ContextHandle, DebuffDamage);
				UAuraAbilitySystemLibrary::SetDebuffDuration(ContextHandle, DebuffDuration);
				UAuraAbilitySystemLibrary::SetDebuffFrequency(ContextHandle, DebuffFrequency);
			}
		}
	}
}

/**
 * @brief 伤害执行计算入口：组装捕获映射、构造评估上下文，计算格挡/护甲/抗性/暴击并输出最终伤害
 *
 * @param ExecutionParams 执行期上下文（包含源/目标 ASC、捕获快照、Owning Spec 等）
 * @param OutExecutionOutput 执行输出容器（把最终伤害写入 IncomingDamage 等属性）
 *
 * 功能说明：
 * - 建立标签到捕获定义的映射，获取源/目标 ASC 与 Avatar 和等级；
 * - 从 Spec 中取聚合标签、SetByCaller 值，组装 EvaluateParameters；
 * - 调用 DetermineDebuff 进行减益判定；
 * - 逐个伤害类型叠加伤害（考虑目标对应抗性），再计算格挡、护甲与护甲穿透、暴击，最后写入输出。
 *
 * 注意事项：
 * - TagsToCaptureDefs 的定义需与 AttributeSet 捕获注册一致（名称/来源/目标/快照时机）；
 * - 读取曲线前请确保指针有效；概率相关比较建议考虑端点（<= 以覆盖 100% 场景）。
 */
void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                              FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// 1) 建立 “标签 -> 捕获定义” 映射（护甲/穿透/格挡/暴击/抗性等）
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;

	// 获取全局标签单例
	const FAuraGamePlayTags& Tags = FAuraGamePlayTags::Get();

	// 次级属性捕获定义
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor,                DamageStatics().ArmorDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_ArmorPenetration,     DamageStatics().ArmorPenetrionDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_BlockChance,          DamageStatics().BlockChanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitChance,    DamageStatics().CriticalHitChanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitResistance,DamageStatics().CriticalHitResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitDamage,    DamageStatics().CriticalHitDamageDef);

	// 元素/物理抗性捕获定义
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Fire,                DamageStatics().FireResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Lightning,           DamageStatics().LightningResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Arcane,              DamageStatics().ArcaneResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Physical,            DamageStatics().PhysicalResistanceDef);

	// 2) 获取源与目标的 ASC
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	// 3) 获取源与目标 Avatar（用于等级与其它接口）
	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	// 通过 CombatInterface 读取双方等级（默认 1）
	int32 SourcePlayerLevel = 1;
	if (SourceAvatar && SourceAvatar->Implements<UCombatInterface>())
	{
		SourcePlayerLevel = ICombatInterface::Execute_GetPlayerLevel(SourceAvatar);
	}
	int32 TargetPlayerLevel = 1;
	if (TargetAvatar && TargetAvatar->Implements<UCombatInterface>())
	{
		TargetPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(TargetAvatar);
	}

	// 4) Owning Spec 与聚合标签
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// 组装评估参数（为捕获求值提供标签上下文）
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	// 5) 先做 Debuff 判定（内部将使用 SetByCaller 与捕获值）
	DetermineDebuff(ExecutionParams, Spec, EvaluateParameters, TagsToCaptureDefs);

	// 6) 计算各伤害类型（按目标对应抗性衰减）并汇总
	float Damage = 0.f;
	for (const TTuple<FGameplayTag, FGameplayTag>& Pair : FAuraGamePlayTags::Get().DamageTypesToResistance)
	{
		const FGameplayTag DamageTag = Pair.Key;    // 伤害类型标签
		const FGameplayTag ResistanceTag = Pair.Value; // 对应抗性标签

		// 映射完整性检查（开发期保障）
		checkf(TagsToCaptureDefs.Contains(ResistanceTag),
			TEXT("ExecCalc_Damage: TagsToCaptureDefs 不包含抗性标签 [%s]"), *ResistanceTag.ToString());

		// 捕获定义（该抗性来自何处并如何求值）
		const FGameplayEffectAttributeCaptureDefinition CaptureDef = TagsToCaptureDefs[ResistanceTag];

		// 读取该类型伤害的 SetByCaller 值（未设置默认 0）
		float DamageTypeValue = Spec.GetSetByCallerMagnitude(DamageTag, false);

		// 读取并求值目标的该类抗性
		float ResistanceValue = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluateParameters, ResistanceValue);

		// 抗性范围限制到 [0, 100]
		ResistanceValue = FMath::Clamp(ResistanceValue, 0.f, 100.f);

		// 按抗性衰减伤害
		DamageTypeValue *= (100.f - ResistanceValue) / 100.f;

		// 汇总
		Damage += DamageTypeValue;
	}

	// 7) 格挡判定：按目标格挡几率减半伤害
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluateParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max(TargetBlockChance, 0.f);

	// 随机格挡判定（可按需要改为 <=）
	const bool bBlockChance = FMath::RandRange(1, 100) < TargetBlockChance;

	// 取出上下文，用于写入“是否被格挡”标记
	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
	UAuraAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, bBlockChance);

	// 被格挡则伤害减半
	Damage = bBlockChance ? Damage / 2.f : Damage;

	// 8) 护甲与护甲穿透
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluateParameters, TargetArmor);
	TargetArmor = FMath::Max(TargetArmor, 0.f);

	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrionDef, EvaluateParameters, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max(SourceArmorPenetration, 0.f);

	// 读取曲线系数（护甲穿透/有效护甲）——按等级评估
	const UCharacterClassInfo* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);
	const FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalculationCoefficientes->FindCurve(FName("ArmorPenetration"), FString());
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourcePlayerLevel);

	// 计算“有效护甲”（穿透降低护甲作用）
	const float EffectiveArmorAfterPen = TargetArmor * (100.f - SourceArmorPenetration * ArmorPenetrationCoefficient) / 100.f;

	const FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalculationCoefficientes->FindCurve(FName("EffectiveArmor"), FString());
	const float EffectiveArmorCoefficient = EffectiveArmorCurve->Eval(TargetPlayerLevel);

	// 按有效护甲系数衰减伤害
	Damage *= (100.f - EffectiveArmorAfterPen * EffectiveArmorCoefficient) / 100.f;

	// 9) 暴击计算：暴击率/暴击伤害/目标暴击抗性（含等级系数）
	float SourceCriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, EvaluateParameters, SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Max(SourceCriticalHitChance, 0.f);

	float SourceCriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageDef, EvaluateParameters, SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max(SourceCriticalHitDamage, 0.f);

	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitResistanceDef, EvaluateParameters, TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max(TargetCriticalHitResistance, 0.f);

	const FRealCurve* CriticalHitResistanceCurve = CharacterClassInfo->DamageCalculationCoefficientes->FindCurve(FName("CriticalHitResistance"), FString());
	const float CriticalHitResistanceCurveCoefficient = CriticalHitResistanceCurve->Eval(TargetPlayerLevel);

	// 有效暴击率 = 暴击率 - 目标抗性 * 等级系数
	const float EffectiveCriticalHitChance = SourceCriticalHitChance - TargetCriticalHitResistance * CriticalHitResistanceCurveCoefficient;

	// 暴击判定
	const bool bIsCriticalHit = FMath::RandRange(1, 100) < EffectiveCriticalHitChance;
	UAuraAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bIsCriticalHit);

	// 暴击伤害：2倍基础伤害 + 额外暴伤（可按项目规则调整）
	Damage = bIsCriticalHit ? 2.f * Damage + SourceCriticalHitDamage : Damage;

	// 10) 写入最终伤害到执行输出（对 IncomingDamage 做加法）
	const FGameplayModifierEvaluatedData EvaluatedData(
		UAuraAttributeSet::GetIncomingDamageAttribute(), // 目标属性：IncomingDamage
		EGameplayModOp::Additive,                        // 叠加方式：加法
		Damage                                           // 最终伤害值
	);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
