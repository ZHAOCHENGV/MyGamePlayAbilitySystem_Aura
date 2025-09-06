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
 * @brief 由本 GA 的“类默认值”生产一次伤害用的参数集（FDamageEffectParams）
 *
 * @param TargetActor                 受击目标（用于计算朝向/击退/死亡冲击方向；可为空则跳过方向推导）
 * @param InRadialDamageOrigin        范围伤害（Radial）原点位置（仅当 bIsRadialDamage 为 true 时使用）
 * @param bOverrideKnockbackDirection 是否手动指定击退方向（true=用 KnockbackDirectionOverride；false=自动朝向目标）
 * @param KnockbackDirectionOverride  手动指定的击退方向（会 Normalize，再乘以 KnockBackForceMagnitude）
 * @param bOverrideDeathImpulse       是否手动指定死亡冲击方向（true=用 DeathImpulseDirectionOverride）
 * @param DeathImpulseDirectionOverride 手动指定的死亡冲击方向（会 Normalize，再乘以 DeathImpulseMagnitude）
 * @param bOverridePitch              是否强制覆盖计算方向的 Pitch（俯仰角）
 * @param PitchOverride               覆盖用的 Pitch 角度（单位：度）
 * @return FDamageEffectParams        填充完成的伤害参数集（供 ApplyDamageEffect 等下游统一使用）
 *
 * 功能说明：
 * - 从本 GA 的 Class 默认配置（伤害/减益/强度等）和运行时传入（目标、方向覆写、范围伤害配置）综合构造一个参数对象；
 * - 方向类参数有两套来源：自动朝向 TargetActor，或使用外部覆写向量（并可选覆盖 Pitch）；
 * - 若启用范围伤害（bIsRadialDamage），会附带半径与原点数据。
 *
 * 详细流程：
 * 1) 初始化 Params 并写入“来源/目标 ASC、GE 类、等级、伤害/减益数值”；
 * 2) 若 TargetActor 有效 → 基于 Avatar→Target 的方向计算击退/死亡冲击（除非被覆写）；
 * 3) 若设置了覆写方向 → 归一化向量并乘以各自强度，可选再用 PitchOverride 调整朝向；
 * 4) 若 bIsRadialDamage → 写入范围伤害开关、原点、内外半径；
 * 5) 返回 Params。
 *
 * 注意事项：
 * - 方向计算分支较多：当 bOverrideX 为 true 时，以覆写方向为准；否则以“指向目标”的向量为准。
 * - Pitch 覆盖是“后处理”：在自动方向和覆写方向两处都可能应用（注意一致性）。
 * - 目标 ASC 可能为 nullptr（比如目标无 ASC），下游应用 GE 时需做好判空。
 * - 范围伤害字段仅在 bIsRadialDamage==true 时有效；建议配合数值表做边界校验（半径>0 等）。
 */
FDamageEffectParams UAuraDamageGameplayAbility::MakeDamageEffectParamsFromClassDefaults(
    AActor* TargetActor,
    FVector InRadialDamageOrigin, bool bOverrideKnockbackDirection, FVector KnockbackDirectionOverride,
    bool bOverrideDeathImpulse, FVector DeathImpulseDirectionOverride, bool bOverridePitch, float PitchOverride) const
{
    FDamageEffectParams Params; // 用于存储所有伤害GE参数的结构体

    Params.WorldContextObject = GetAvatarActorFromActorInfo(); // 技能释放者的Actor指针，作为上下文
    Params.DamageGameplayEffectClass = DamageEffectClass; // 当前技能对应的伤害GE类型
    Params.SourceAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo(); // 技能释放者ASC
    Params.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor); // 目标的ASC（可能为空）
    Params.BaseDamage = Damage.GetValueAtLevel(GetAbilityLevel()); // 按当前技能等级查表获取基础伤害（FScalableFloat）
    Params.AbilityLevel = GetAbilityLevel(); // 当前技能等级
    Params.DamageType = DamageType; // 伤害类型标签（如物理/火焰/冰冻）
    Params.DebuffChance = DebuffChance; // 减益触发概率
    Params.DebuffDamage = DebuffDamage; // 减益每跳伤害
    Params.DebuffDuration = DebuffDuration; // 减益持续时间
    Params.DebuffFrequency = DebuffFrequency; // 减益触发频率（周期秒）
    Params.DeathImpulseMagnitude = DeathImpulseMagnitude; // 死亡冲击强度
    Params.KnockBackForceMagnitude = KnockBackForceMagnitude; // 击退力度
    Params.KnockBackChance = KnockBackChance; // 击退几率

    // 若目标有效：用“Avatar→Target”的方向作为基础方向（除非后续覆写）
    if (IsValid(TargetActor))
    {
        FRotator Rotation = (TargetActor->GetActorLocation() - GetAvatarActorFromActorInfo()->GetActorLocation()).Rotation(); // 指向目标的朝向
        if (bOverridePitch)
        {
            Rotation.Pitch = PitchOverride; // 覆盖 Pitch（俯仰角）
        }
        const FVector ToTarget = Rotation.Vector(); // 方向向量（单位向量）

        if (!bOverrideKnockbackDirection) // 未覆写击退方向 → 用自动方向
        {
            Params.KnockBackForce = ToTarget * KnockBackForceMagnitude; // 击退向量 = 方向 * 强度
        }
        if (!bOverrideDeathImpulse) // 未覆写死亡冲击方向 → 用自动方向
        {
            Params.DeathImpulse = ToTarget * DeathImpulseMagnitude; // 死亡冲击向量 = 方向 * 强度
        }
    }

    // 若覆写击退方向：Normalize 后乘以力度；可选再用 Pitch 覆盖
    if (bOverrideKnockbackDirection)
    {
        KnockbackDirectionOverride.Normalize(); // 归一化，防止强度被方向长度影响
        Params.KnockBackForce = KnockbackDirectionOverride * KnockBackForceMagnitude; // 基础按向量覆写

        if (bOverridePitch)
        {
            FRotator KnockbackRotation = KnockbackDirectionOverride.Rotation(); // 从向量转朝向
            KnockbackRotation.Pitch = PitchOverride; // 覆盖 Pitch
            Params.KnockBackForce = KnockbackRotation.Vector() * KnockBackForceMagnitude; // 以覆盖后的朝向重新计算向量
        }
    }

    // 若覆写死亡冲击方向：Normalize 后乘以强度；可选再用 Pitch 覆盖
    if (bOverrideDeathImpulse)
    {
        DeathImpulseDirectionOverride.Normalize(); // 归一化
        Params.DeathImpulse = DeathImpulseDirectionOverride * DeathImpulseMagnitude; // 基础按向量覆写

        if (bOverridePitch)
        {
            FRotator DeathImpulseRotation = DeathImpulseDirectionOverride.Rotation(); // 从向量转朝向
            DeathImpulseRotation.Pitch = PitchOverride; // 覆盖 Pitch
            Params.DeathImpulse = DeathImpulseRotation.Vector() * KnockBackForceMagnitude; // 用 Pitch 后的朝向*（此处乘以 KnockBack 力度；见下方“潜在笔误”）
        }
    }

    // 若启用范围伤害：写入相关参数
    if (bIsRadialDamage)
    {
        Params.bIsRadialDamage = bIsRadialDamage; // 标记启用范围伤害
        Params.RadialDamageOrigin = InRadialDamageOrigin; // 原点（通常是命中点/投射物爆炸点）
        Params.RadialDamageInnerRadius = RadialDamageInnerRadius; // 内半径（满额伤害区）
        Params.RadialDamageOuterRadius = RadialDamageOuterRadius; // 外半径（衰减到 0 的边界）
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