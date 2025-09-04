// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/AuraDamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

/**
 * @brief 对目标Actor施加伤害效果（通用伤害应用）
 *
 * @param TargetActor 需要受到伤害的目标Actor（如敌人、怪物、Boss等）
 *
 * 功能说明：
 * 本函数是所有伤害型技能的核心调用逻辑。它通过虚幻GAS系统的GameplayEffect，自动计算并施加伤害数值，
 * 适用于近战攻击、远程技能、法术等场景。
 *
 * 详细流程：
 * 1. 创建一个伤害类型的GameplayEffect规格（Spec），指定伤害类型和等级。
 * 2. 按照当前技能等级，查表获得实际伤害数值，可实现技能成长/缩放。
 * 3. 使用SetByCaller机制，将伤害类型和伤害值动态注入GE，便于蓝图和数据驱动。
 * 4. 获取目标的AbilitySystemComponent（ASC），实现伤害/属性变更和网络同步。
 *
 * 注意事项：
 * - TargetActor 必须实现AbilitySystemComponent，否则伤害不会生效。
 * - DamageEffectClass和DamageType务必提前配置好。
 * - 适用于单体伤害，如需群体伤害可循环调用本函数。
 */
void UAuraDamageGameplayAbility::CauseDamage(AActor* TargetActor)
{
    // 1. 创建伤害GE规格（指定类型和等级）
    FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, 1.f);

    // 2. 按技能等级查表获取伤害（支持成长）
    const float ScaleDamage = Damage.GetValueAtLevel(GetAbilityLevel());

    // 3. 通过SetByCaller机制注入伤害类型和数值
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, DamageType, ScaleDamage);

    // 4. 获取目标的AbilitySystemComponent，并施加伤害GE
    UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo(); // 技能释放者ASC
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor); // 目标ASC

    // 检查指针有效性，避免崩溃
    if (SourceASC && TargetASC && DamageSpecHandle.Data.IsValid())
    {
        SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), TargetASC); // 应用伤害，属性自动同步
    }
    // 建议：可加日志Debug TargetASC为空时的情况，方便排查
}

/**
 * @brief 从当前技能默认配置生成减益 伤害效果参数结构体
 *
 * @param TargetActor 伤害技能的目标Actor（通常是敌人或玩家）
 * @return FDamageEffectParams 结构体，包含所有后续执行伤害GE所需的参数
 *
 * 功能说明：
 * 本函数用于组装一次伤害技能所需的全部参数，便于后续统一传递和调用。适用于多种伤害相关GE的生成。
 *
 * 详细流程：
 * 1. 获取技能释放者（自身）的Actor指针作为上下文（WorldContextObject）。
 * 2. 记录当前技能默认的伤害GE类型。
 * 3. 获取技能释放者和目标的AbilitySystemComponent（ASC），便于后续GE应用。
 * 4. 读取当前技能等级下的基础伤害。
 * 5. 记录技能等级、伤害类型以及所有与减益相关的参数（如概率、数值、持续时间、频率）。
 *
 * 注意事项：
 * - DamageEffectClass、DamageType、Debuff相关参数需在技能蓝图或C++中配置好。
 * - TargetActor 必须能获取到ASC，否则后续GE无法正确应用。
 * - 便于扩展，后续如有新参数可在此结构体和赋值处补充。
 */
FDamageEffectParams UAuraDamageGameplayAbility::MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor) const
{
    FDamageEffectParams Params; // 用于存储所有伤害GE参数的结构体

    Params.WorldContextObject = GetAvatarActorFromActorInfo(); // 技能释放者的Actor指针，作为上下文
    Params.DamageGameplayEffectClass = DamageEffectClass; // 当前技能对应的伤害GE类型
    Params.SourceAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo(); // 技能释放者ASC
    Params.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor); // 目标的ASC
    Params.BaseDamage = Damage.GetValueAtLevel(GetAbilityLevel()); // 按当前技能等级查表获取基础伤害
    Params.AbilityLevel = GetAbilityLevel(); // 当前技能等级
    Params.DamageType = DamageType; // 伤害类型标签（如物理/火焰/冰冻）
    Params.DebuffChance = DebuffChance; // 减益（debuff）触发概率
    Params.DebuffDamage = DebuffDamage; // 减益每跳伤害
    Params.DebuffDuration = DebuffDuration; // 减益持续时间
    Params.DebuffFrequency = DebuffFrequency; // 减益触发频率
    Params.DeathImpulseMagnitude = DeathImpulseMagnitude;// 死亡冲击强度
    Params.KnockBackForceMagnitude = KnockBackForceMagnitude;// 击退力度
    Params.KnockBackChance = KnockBackChance; //击退几率
    if (IsValid(TargetActor))
    {
        FRotator Rotation = (TargetActor->GetActorLocation() - GetAvatarActorFromActorInfo()->GetActorLocation()).Rotation();
        Rotation.Pitch = 45.f;
        const FVector ToTarget = Rotation.Vector();
        Params.DeathImpulse = ToTarget * DeathImpulseMagnitude;
        Params.KnockBackForce = ToTarget * KnockBackForceMagnitude;
    }

    if (bIsRadialDamage)
    {
        Params.bIsRadialDamage = bIsRadialDamage;
        Params.RadialDamageOrigin = RadialDamageOrigin;
        Params.RadialDamageInnerRadius = RadialDamageInnerRadius;
        Params.RadialDamageOuterRadius = RadialDamageOuterRadius;
    }
    return Params; // 返回完整参数结构体，便于后续统一使用
}

float UAuraDamageGameplayAbility::GetDamageAtLevel() const
{
    return Damage.GetValueAtLevel(GetAbilityLevel());
}


/**
 * @brief 从动画蒙太奇数组中随机获取一个带标签的动画
 *
 * @param TaggedMontages 储存动画与标签的结构体数组
 * @return 随机返回一个FTaggedMontage，如数组为空则返回默认值
 *
 * 功能说明：
 * 本函数常用于技能动画池。每次调用会在可用动画中随机抽一个，提升技能表现多样性。
 *
 * 详细流程：
 * 1. 判断数组是否非空，避免越界崩溃。
 * 2. 随机选取一个动画索引，返回对应FTaggedMontage。
 * 3. 如果数组为空，返回默认构造体（代表无动画）。
 *
 * 注意事项：
 * - 动画池可根据需求扩展为加权随机或类型筛选。
 * - 如果技能没有动画可以直接返回默认值。
 */
FTaggedMontage UAuraDamageGameplayAbility::GetRandomTaggedMontageFromArray(
    const TArray<FTaggedMontage>& TaggedMontages) const
{
    if (TaggedMontages.Num() > 0) // 判断数组非空
    {
        const int32 Selection = FMath::RandRange(0, TaggedMontages.Num() - 1); // 随机选索引
        return TaggedMontages[Selection]; // 返回选中的动画
    }
    return FTaggedMontage(); // 数组为空时返回默认值
}