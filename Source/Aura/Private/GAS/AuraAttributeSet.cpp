// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AuraAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AuraGamePlayTags.h"
#include "GameplayEffectExtension.h"
#include "Aura/AuraLogChannels.h"
#include "GameFramework/Character.h"
#include "GAS/AuraAbilitySystemLibrary.h"
#include "Interation/CombatInterface.h"
#include "Interation/PlayerInterface.h"
#include "Net/UnrealNetwork.h"
#include "Player/AuraPlayerController.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UAuraAttributeSet::UAuraAttributeSet()
{
	//FAuraGamePlayTags::Get()获取游戏唯一实列  并引用游戏中定义的所有游戏标签
	const FAuraGamePlayTags& GamePlayTags = FAuraGamePlayTags::Get();
	
	//重要属性
	TagsToAttributes.Add(GamePlayTags.Attributes_Primary_Strength, GetStrengthAttribute);
	TagsToAttributes.Add(GamePlayTags.Attributes_Primary_Intelligence, GetIntelligenceAttribute);
	TagsToAttributes.Add(GamePlayTags.Attributes_Primary_Resilience, GetResilienceAttribute);
	TagsToAttributes.Add(GamePlayTags.Attributes_Primary_Vigor, GetVigorAttribute);
	
	//次要属性
	TagsToAttributes.Add(GamePlayTags.Attributes_Secondary_Armor, GetArmorAttribute);
	TagsToAttributes.Add(GamePlayTags.Attributes_Secondary_ArmorPenetration, GetArmorPenetrionAttribute);
	TagsToAttributes.Add(GamePlayTags.Attributes_Secondary_BlockChance, GetBlockChanceAttribute);
	TagsToAttributes.Add(GamePlayTags.Attributes_Secondary_CriticalHitChance, GetCriticalHitChanceAttribute);
	TagsToAttributes.Add(GamePlayTags.Attributes_Secondary_CriticalHitResistance, GetCriticalHitResistanceAttribute);
	TagsToAttributes.Add(GamePlayTags.Attributes_Secondary_CriticalHitDamage, GetCriticalHitDamageAttribute);
	TagsToAttributes.Add(GamePlayTags.Attributes_Secondary_HealthRegeneration, GetHealthRegenerationAttribute);
	TagsToAttributes.Add(GamePlayTags.Attributes_Secondary_ManaRegeneration, GetManaRegenerationAttribute);
	TagsToAttributes.Add(GamePlayTags.Attributes_Secondary_MaxHealth, GetMaxHealthAttribute);
	TagsToAttributes.Add(GamePlayTags.Attributes_Secondary_MaxMana, GetMaxManaAttribute);

	/*
	 * 伤害类型抗性属性
	 */
	TagsToAttributes.Add(GamePlayTags.Attributes_Resistance_Fire, GetFireResistanceAttribute);
	TagsToAttributes.Add(GamePlayTags.Attributes_Resistance_Lightning, GetLightningResistanceAttribute);
	TagsToAttributes.Add(GamePlayTags.Attributes_Resistance_Arcane, GetArcaneResistanceAttribute);
	TagsToAttributes.Add(GamePlayTags.Attributes_Resistance_Physical, GetPhysicalResistanceAttribute);
}

void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// DOREPLIFETIME_CONDITION_NOTIFY宏 是注册属性的网络同步规则
	// COND_None: 无任何额外的同步条件，始终同步该属性
	// REPNOTIFY_Always: 每次同步该属性时，总是触发 RepNotify 函数（例如 OnRep_Health）

	//重要属性
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,Health,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,Mana,COND_None,REPNOTIFY_Always);

	//主要属性
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,Strength,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,Intelligence,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,Resilience,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,Vigor,COND_None,REPNOTIFY_Always);

	//次要属性
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,Armor,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,ArmorPenetrion,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,BlockChance,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,CriticalHitChance,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,CriticalHitDamage,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,CriticalHitResistance,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,HealthRegeneration,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,ManaRegeneration,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,MaxHealth,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,MaxMana,COND_None,REPNOTIFY_Always);

	/*
	 * 伤害类型抗性属性
	 */
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,FireResistance,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,LightningResistance,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,ArcaneResistance,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,PhysicalResistance,COND_None,REPNOTIFY_Always);

}

/**
 * @brief 在 Attribute 值即将发生变化前进行预裁剪（如 Health/Mana 保持在 [0, Max]）
 * @param Attribute 本次即将变化的属性（如 Health 或 Mana）
 * @param NewValue  引擎准备写入的新值（可在此修改以生效）
 * @details
 *  - 【作用】在引擎把 NewValue 写入 Attribute 之前，先做一次边界处理，避免出现负值或超过上限。
 *  - 【背景】GAS 在属性写入生命周期中会调用 PreAttributeChange → 实际写入 → PostAttributeChange 等钩子。
 *  - 【流程】1) 调用父类；2) 若是 Health，则 Clamp 到 [0, MaxHealth] 并日志；3) 若是 Mana，同理。
 *  - 【注意】这里是“预改值”阶段，尽量只做轻量校验；复杂逻辑可放到 PostGameplayEffectExecute 等阶段。
 */
void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	// 步骤 1：保持引擎默认流程
	Super::PreAttributeChange(Attribute, NewValue); // 调父类

	// 步骤 2：若本次改的是 Health，则限制到 [0, MaxHealth]
	if (Attribute == GetHealthAttribute())
	{
		// Clamp 新值到合法范围（防止负血/超过上限）
		NewValue = FMath::Clamp(NewValue,0,GetMaxHealth()); // 0 ≤ Health ≤ MaxHealth
		UE_LOG(LogTemp,Warning,TEXT("Health : %f"),NewValue); // 打印新值
	}
	
	// 步骤 3：若本次改的是 Mana，则限制到 [0, MaxMana]
	if (Attribute == GetManaAttribute())
	{
		// Clamp 新值到合法范围（防止负蓝/超过上限）
		NewValue = FMath::Clamp(NewValue,0,GetMaxMana()); // 0 ≤ Mana ≤ MaxMana
		UE_LOG(LogTemp,Warning,TEXT("Mana : %f"),NewValue); // 打印新值
	}
}

/**
 * @brief 在 GE 修改实际“落地”到属性后进行处理（血量/蓝量修正、伤害与经验结算等）
 * @param Data GE 修改回调数据（包含被改的 Attribute、改动来源/目标等上下文）
 * @details
 *  - 【作用】当 GE 修改完成（已写入属性）后，根据是哪一个 Attribute 被修改，执行后续逻辑（如结算伤害/经验）。
 *  - 【流程】
 *    1) 调用父类与组装 EffectProperties；
 *    2) 若目标已死亡则提前返回；
 *    3) 若 Health/Mana 被改，做一次 Clamp 与日志；
 *    4) 若 IncomingDamage 到达，调用 HandleIncomingDamage 结算；
 *    5) 若 IncomingXP 到达，调用 HandleIncomingXP 结算。
 *  - 【注意】此时属性值已更新；适合触发“依赖当前值”的行为（如死亡判定/击退/命中特效等）。
 */
void UAuraAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	// 步骤 1：保持引擎默认行为
	Super::PostGameplayEffectExecute(Data); // 调父类

	// 步骤 2：汇总常用上下文（Source/Target/ASC/Avatar/Character/ContextHandle 等）
	FEffectProperties Props; // 临时容器
	SetEffectProperties(Data,Props); // 从 Data 填充 Props

	// 步骤 3：若目标已死亡（通过 CombatInterface 查询），则不再处理后续逻辑
	if (Props.TargetCharacter->Implements<UCombatInterface>() && ICombatInterface::Execute_IsDead(Props.TargetCharacter))return; // 已死 → 返回
	
	// 步骤 4：若本次被改的是 Health，则再次 Clamp 并输出日志
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// 再次将 Health 约束到 [0, MaxHealth]
		SetHealth(FMath::Clamp(GetHealth(),0,GetMaxHealth())); // 防止越界
		UE_LOG(LogTemp,Warning,TEXT(" %s 的剩余血量为 : %f"), *Props.TargetAvatarActor->GetName(), GetHealth()); // 打印
	}
	// 步骤 5：若本次被改的是 Mana，则再次 Clamp
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(),0,GetMaxMana())); // 约束蓝量
	}
	// 步骤 6：若本次落地的是 IncomingDamage（“将要结算的伤害”池），则走伤害结算流程
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		HandleIncomingDamage(Props); // 伤害结算与受击表现
	}
	
	// 步骤 7：若本次落地的是 IncomingXP，则走经验结算流程
	if (Data.EvaluatedData.Attribute == GetIncomingXPAttribute())
	{
		HandleIncomingXP(Props); // 经验结算与升级
	}
}

/**
 * @brief 处理刚落地的伤害：扣血、死亡判定、击退/受击反应、伤害飘字、可能触发 Debuff
 * @param Props 上下文（Source/Target/ASC/Avatar/Character/EffectContextHandle 等）
 * @details
 *  - 【流程】
 *    1) 读取 IncomingDamage 并清零（消费这次伤害）；
 *    2) 若伤害>0：计算新血量并 Clamp；若 ≤0 则忽略；
 *    3) 若致死：调用 Die()，并发送经验事件；
 *       否则：激活“受击反应”GA，按 Context 的 KnockBackForce 做击退；
 *    4) 读 Block/Critical 状态，显示飘字；
 *    5) 若 Context 标记本次 Debuff 成功，则调用 Debuff() 追加 DoT。
 *  - 【注意】这里假设 IncomingDamage 已由 GE/Execution 写入（例如你的 DoT 执行体每周期加一次）。
 */
void UAuraAttributeSet::HandleIncomingDamage(const FEffectProperties& Props)
{
	// 步骤 1：读取本次要结算的伤害（局部缓存）
	const float LocalIncomingDamage = GetIncomingDamage(); // 取值

	// 步骤 2：将 IncomingDamage 清零（避免重复结算）
	SetIncomingDamage(0.f); // 消费

	// 步骤 3：仅当伤害>0 时才进行后续处理
	if (LocalIncomingDamage > 0)
	{
	const float NewHealth = GetHealth() - LocalIncomingDamage; // 扣血
	
		// 步骤 3.2：写回并 Clamp 到 [0, MaxHealth]
		SetHealth(FMath::Clamp(NewHealth,0,GetMaxHealth())); // 防止越界
	
		// 步骤 3.3：致死判定（≤0）
		const bool bFatal = NewHealth <= 0.f; // 是否致死
		if (bFatal)
		{
			// 步骤 3.3.1：死亡处理（待办：死亡冲击等）
			ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor); // 目标接口
			if (CombatInterface)
			{
				CombatInterface->Die(); // 触发死亡
			}
			// 步骤 3.3.2：给来源发经验（如有）
			SendXPEvent(Props); // 经验事件（由上层实现）
		}
		else
		{
			//如果未在眩暈中
			if (Props.TargetCharacter->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsBeingShocked(Props.TargetCharacter))
			{
				// 步骤 3.4：未致死 → 触发“受击反应”GA（命中反应）
				FGameplayTagContainer TagContainer; // 标签容器
				TagContainer.AddTag(FAuraGamePlayTags::Get().Effects_HitReact); // 添加受击标签
				Props.TargetASC->TryActivateAbilitiesByTag(TagContainer); // 尝试激活对应 GA
			}
			

			// 步骤 3.5：从 Context 读取击退向量；非零则 LaunchCharacter
			const FVector& KnockBackForce = UAuraAbilitySystemLibrary::GetKnockBackForce(Props.EffectContextHandle); // 击退力
			if (!KnockBackForce.IsNearlyZero(1.f)) // 大小阈值判定
			{
				Props.TargetCharacter->LaunchCharacter(KnockBackForce, true,true); // 发射（XY/Z 是否覆盖速度）
			}
		}

		// 步骤 4：读取格挡/暴击标记，用于飘字表现
		const bool bBlock = UAuraAbilitySystemLibrary::IsBlockedHit(Props.EffectContextHandle); // 是否格挡
		const bool bCriticalHit = UAuraAbilitySystemLibrary::IsCriticalHit(Props.EffectContextHandle); // 是否暴击
		ShowFloatingText(Props, LocalIncomingDamage, bBlock, bCriticalHit); // 飘字：伤害/格挡/暴击

		// 步骤 5：若本次 Debuff 判定成功，则追加 Debuff（如 DoT）
		if (UAuraAbilitySystemLibrary::IsSuccessfulDebuff(Props.EffectContextHandle)) // Debuff 命中？
		{
			Debuff(Props); // 施加 Debuff（内部创建周期 GE）
		}
	}
}
void UAuraAttributeSet::HandleIncomingXP(const FEffectProperties& Props)
{

	const float LocalIncomingXP = GetIncomingXP();

	SetIncomingXP(0.f);

	UE_LOG(LogAura,Log,TEXT(" Incoming XP : %f"),LocalIncomingXP );

		

	if (Props.SourceCharacter->Implements<UPlayerInterface>() && Props.SourceCharacter->Implements<UCombatInterface>())
	{
	
		const int32 CurrentLevel = ICombatInterface::Execute_GetPlayerLevel(Props.SourceCharacter);
	
		const int32 CurrentXP = IPlayerInterface::Execute_GetXP(Props.SourceCharacter);
		
		const int32 NewLevel = IPlayerInterface::Execute_FindLevelForXP(Props.SourceCharacter, CurrentXP + LocalIncomingXP);
	
		const int32 NumLevelUps = NewLevel - CurrentLevel;
	
		if (NumLevelUps > 0)
		{
			const int32 AttributePointsReward = IPlayerInterface::Execute_GetAttributePointsReward(Props.SourceCharacter, CurrentLevel);
			const int32 SpellPointsReward = IPlayerInterface::Execute_GetSpellPointsReward(Props.SourceCharacter, CurrentLevel);

			IPlayerInterface::Execute_AddToPlayerLevel(Props.SourceCharacter,NumLevelUps);
			IPlayerInterface::Execute_AddToAttributePoints(Props.SourceCharacter,AttributePointsReward);
			IPlayerInterface::Execute_AddToSpellPoints(Props.SourceCharacter,SpellPointsReward);

		
			bTopOffHealth = true;
			bTopOffMana = true;
				
				
				

		
			IPlayerInterface::Execute_LeveUp(Props.SourceCharacter);
		}
		
		IPlayerInterface::Execute_AddToXP(Props.SourceCharacter, LocalIncomingXP);
	}
}


/**
 * @brief 从上游 EffectContext 读取 Debuff 参数，动态创建“周期伤害（DoT）”GE 并施加到 TargetASC
 * @param Props 作用参数集合（SourceASC/TargetASC、SourceAvatarActor、EffectContextHandle 等）
 * @return void 无返回（副作用：向 TargetASC 应用一个有持续时间、按周期触发的 UGameplayEffect）
 *
 * 背景知识（UE/GAS）：
 * - UGameplayEffect（GE）是“配方”，FGameplayEffectSpec（Spec）是“处方实例”；Period>0 表示按固定时间间隔触发周期逻辑。
 * - 默认情况下，GE 的周期逻辑会在应用时立刻执行一次（bExecutePeriodicEffectOnApplication=true）；若想“首跳延后”，需设为 false。
 * - **Modifiers**（修饰器）是“立即生效”的属性改动：GE 一应用就对 Attribute 产生影响；与“周期触发”无关。
 * - 典型 DoT 推荐用 **Execution**（UGameplayEffectExecutionCalculation）在每个周期里结算伤害，而不是用 Modifier 直接加伤害。
 *
 * 详细流程：
 * 1) 从单例获取项目 GameplayTags；用 SourceASC 创建 EffectContext，并记录 SourceAvatarActor 为 SourceObject。
 * 2) 从 Props.EffectContextHandle 读取 DamageType / DebuffDamage / DebuffDuration / DebuffFrequency。
 * 3) New 一个临时 GE：配置 Duration/Period，并将 bExecutePeriodicEffectOnApplication 设为 false（首跳延后）。
 * 4) 通过 UTargetTagsGameplayEffectComponent 给目标添加 Debuff 相关 Tag；设置叠加策略与上限。
 * 5) 向 GE 添加一个对 IncomingDamage 的 Modifier（当前写法：立刻对伤害做加法，注意这会“应用即生效”）。
 * 6) new 一个 FGameplayEffectSpec，向自定义 Context 回写 DamageType，并将 Spec 施加到 TargetASC。
 *
 * 注意事项：
 * - 即使 bExecutePeriodicEffectOnApplication=false，也**无法阻止 Modifier 的“即时生效”**——这正是你看到“刚应用就跳伤害”的根因之一。
 * - 若希望“首跳严格在 1 秒后”，请不要用 Modifier 对伤害直接加值，改为在 **Execution** 中按周期结算（见本文后“修复思路”）。
 * - DamageTypesToDebuffs[DamageType] 使用前应确保映射存在；DebuffFrequency>0、DebuffDuration>=0 建议做边界检查。
 */
void UAuraAttributeSet::Debuff(const FEffectProperties& Props)
{
	// 取项目统一的 GameplayTags（含各种伤害/减益映射）
	const FAuraGamePlayTags& GamePlayTags = FAuraGamePlayTags::Get();

	// 用 SourceASC 创建 EffectContext（处方上下文，携带施加来源信息）
	FGameplayEffectContextHandle EffectContext= Props.SourceASC->MakeEffectContext();

	// 把 SourceAvatarActor 记录为 SourceObject（便于 Cue / 归因）
	EffectContext.AddSourceObject(Props.SourceAvatarActor);

	// 从上游 Context 读取伤害类型（FGameplayTag）
	const FGameplayTag DamageType = UAuraAbilitySystemLibrary::GetDamageType(Props.EffectContextHandle);

	// 读取每跳伤害（float）
	const float DebuffDamage = UAuraAbilitySystemLibrary::GetDebuffDamage(Props.EffectContextHandle);

	// 读取总持续时间（秒）
	const float DebuffDuration = UAuraAbilitySystemLibrary::GetDebuffDuration(Props.EffectContextHandle);

	// 读取周期间隔（秒/次）
	const float DebuffFrequency = UAuraAbilitySystemLibrary::GetDebuffFrequency(Props.EffectContextHandle);

	// 拼接临时 GE 的可读名称（便于日志/调试）
	FString DebuffName = FString::Printf(TEXT("DynamicDebuff_%s"),*DamageType.ToString());

	// 在 Transient 包中创建一次性 UGameplayEffect（仅供本次应用）
	UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(DebuffName));

	// 设置为“有持续时间”的 GE
	Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration;

	// 设置周期间隔（>0 才会周期触发）
	Effect->Period = DebuffFrequency;

	// 设置持续时间（可伸缩浮点包装）
	Effect->DurationMagnitude = FScalableFloat(DebuffDuration);

	// 关键：首跳不在应用瞬间执行（仅对“周期执行体/Execution”生效；对 Modifier 不起作用）
	Effect->bExecutePeriodicEffectOnApplication = false;  

	// 构造目标标签容器（通过组件式 API 添加到目标）
	FInheritedTagContainer TagContainer = FInheritedTagContainer();
	
	// 找到或添加 UTargetTagsGameplayEffectComponent（用来操作目标的 Tag 变化）
	UTargetTagsGameplayEffectComponent& Component = Effect->FindOrAddComponent<UTargetTagsGameplayEffectComponent>();


	const FGameplayTag DebuffTag = GamePlayTags.DamageTypesToDebuffs[DamageType];
	// 将“伤害类型 → Debuff 标签”的映射结果加入容器
	TagContainer.Added.AddTag(DebuffTag);
	// 如果Debuff标签为 晕眩 时
	if (DebuffTag.MatchesTagExact(GamePlayTags.Debuff_Stun))
	{
		//将“ 阻止光标追踪，移动，输入已按下，输入已松开等 Debuff 标签”的映射结果加入容器，阻止这些效果 
		TagContainer.Added.AddTag(GamePlayTags.Player_Block_CursorTrace);
		TagContainer.Added.AddTag(GamePlayTags.Player_Block_InputHeld);
		TagContainer.Added.AddTag(GamePlayTags.Player_Block_InputPressed);
		TagContainer.Added.AddTag(GamePlayTags.Player_Block_InputReleased);
	}
	
	
	// 应用到目标标签（使目标获得对应的 Debuff Tag）
	Component.SetAndApplyTargetTagChanges(TagContainer);

	// 叠加策略：同一来源聚合
	Effect->StackingType = EGameplayEffectStackingType::AggregateBySource;

	// 限制最大叠加层数为 1
	Effect->StackLimitCount = 1;

	// 记录修饰器数组当前尾索引
	const int32 Index = Effect->Modifiers.Num();                          

	// 向 GE 追加一条默认修饰器（FGameplayModifierInfo）
	Effect->Modifiers.Add(FGameplayModifierInfo());                    

	// 引用刚刚加入的修饰器
	FGameplayModifierInfo& ModifierInfo = Effect->Modifiers[Index];      
	
	// 设置修饰器幅度 = 每跳伤害（注意：Modifier 会在“应用时立刻生效”，与周期无关）
	ModifierInfo.ModifierMagnitude = FScalableFloat(DebuffDamage);

	// 修饰器为加法叠加
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;

	// 修饰的目标属性是 IncomingDamage（常见做法：作为即将结算的伤害池）
	ModifierInfo.Attribute = UAuraAttributeSet::GetIncomingDamageAttribute();

	// 构造一个 FGameplayEffectSpec（实例），等级 1.f；用前面的 Effect 与 Context
	if (FGameplayEffectSpec* MutableSpec = new FGameplayEffectSpec(Effect, EffectContext, 1.f)) 
	{
		// 从 Spec 获取我们扩展的 FAuraGameplayEffectContext 指针
		FAuraGameplayEffectContext* AuraContext = static_cast<FAuraGameplayEffectContext*>(MutableSpec->GetContext().Get());
	
		// 准备共享指针包装的 DamageType（匹配 Context 字段类型）
		TSharedPtr<FGameplayTag> DebuffDamageType = MakeShareable(new FGameplayTag(DamageType));

		// 回写 DamageType 到 Context（下游读取一致）
		AuraContext->SetDamageType(DebuffDamageType);

		// 对 TargetASC 应用该 Spec（GE 生效：Modifier 立即作用；周期逻辑将从下一个周期点开始）
		Props.TargetASC->ApplyGameplayEffectSpecToSelf(*MutableSpec);
	}
}

void UAuraAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute() && bTopOffHealth)
	{
		UE_LOG(LogAura,Log,TEXT(" 当前HP : %f"),GetHealth());
		SetHealth(GetMaxHealth());
		UE_LOG(LogAura,Log,TEXT(" 升级后HP : %f"),GetHealth());
		bTopOffHealth = false;
	}
	if (Attribute == GetMaxManaAttribute() && bTopOffMana)
	{
		SetMana(GetMaxMana());
		bTopOffMana = false;
	}
}

/**
 * 在客户端显示浮动伤害数字文本
 * 
 * @param Props          效果属性结构体，包含源角色和目标角色信息
 * @param Damage        要显示的伤害值（可正可负，正值为伤害，负值为治疗）
 * @param bBlockedHit   是否为被格挡的攻击
 * @param bCriticalHit  是否为暴击攻击
 * 
 * @note 显示规则：
 * - 当攻击者与目标不是同一角色时触发显示
 * - 优先在攻击者控制器显示（适用于玩家主动攻击的情况）
 * - 当攻击者是AI时，在目标玩家控制器显示（适用于玩家被AI攻击的情况）
 * - 典型应用场景：多人游戏中伤害数字的本地化显示
 */
void UAuraAttributeSet::ShowFloatingText(const FEffectProperties& Props, float Damage, bool bBlockedHit, bool bCriticalHit) const
{
	// 仅在不同源角色和目标角色之间显示
	if(Props.SourceCharacter != Props.TargetCharacter)
	{
		// 优先尝试获取攻击者的玩家控制器
		if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(Props.SourceCharacter->Controller))
		{
			// 在攻击者客户端显示伤害数字（玩家主动发起攻击时）
			PC->ShowDamageNumber(Damage,Props.TargetCharacter, bBlockedHit, bCriticalHit);
			// 已显示则提前返回
			return;
		}
		// 当攻击者是AI时，尝试在目标玩家的控制器显示
		if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(Props.TargetCharacter->Controller))
		{
			// 在受击者客户端显示伤害数字（玩家被攻击时）
			PC->ShowDamageNumber(Damage,Props.TargetCharacter, bBlockedHit, bCriticalHit);
		}
	}
}

/**
 * 发送经验值奖励事件到来源角色
 * @param Props 效果属性结构体，包含来源和目标角色的相关信息
 * 
 * 功能流程：
 * 1. 验证目标角色是否实现战斗接口
 * 2. 计算基于目标等级和职业的经验奖励
 * 3. 构建并发送游戏事件到来源角色
 */
void UAuraAttributeSet::SendXPEvent(const FEffectProperties& Props)
{
	// 尝试将目标角色转换为战斗接口（用于获取等级和职业信息）
	if(Props.TargetCharacter->Implements<UCombatInterface>())
	{
		// 获取目标的等级
		const int32 TargetLevel = ICombatInterface::Execute_GetPlayerLevel(Props.TargetCharacter);
		// 获取目标的职业(调用C++版本函数）
		const ECharacterClass TargetClass = ICombatInterface::Execute_GetCharacterClass(Props.TargetCharacter);
		// 获取该等级和职业对应的经验奖励值
		const int32 XPReward = UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(Props.TargetCharacter,TargetClass,TargetLevel);
		// 获取游戏标签单例
		const FAuraGamePlayTags& GamePlayTags = FAuraGamePlayTags::Get();
		// 构建事件数据包
		FGameplayEventData Payload;
		Payload.EventTag = GamePlayTags.Attributes_Meta_IncomingXP;
		Payload.EventMagnitude = XPReward;
		// 发送游戏事件到来源角色
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Props.SourceCharacter,GamePlayTags.Attributes_Meta_IncomingXP,Payload);
	}
}



void UAuraAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,Armor,OldArmor);
}

void UAuraAttributeSet::OnRep_ArmorPenetrion(const FGameplayAttributeData& OldArmorPenetrion) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,ArmorPenetrion,OldArmorPenetrion);
}

void UAuraAttributeSet::OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,BlockChance,OldBlockChance);
}

void UAuraAttributeSet::OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,CriticalHitChance,OldCriticalHitChance);
}

void UAuraAttributeSet::OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,CriticalHitDamage,OldCriticalHitDamage);
}

void UAuraAttributeSet::OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,CriticalHitResistance,OldCriticalHitResistance);
}

void UAuraAttributeSet::OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,HealthRegeneration,OldHealthRegeneration);
}

void UAuraAttributeSet::OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,ManaRegeneration,OldManaRegeneration);
}

void UAuraAttributeSet::SetEffectProperties(const struct FGameplayEffectModCallbackData& Data, FEffectProperties& Props)const
{
	// 获取Gameplay Effect的上下文句柄
	Props.EffectContextHandle = Data.EffectSpec.GetContext();
	// 获取原始施加效果的Ability System Component
 	Props.SourceASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();
	// 检查SourceASC和相关的Actor是否有效
	if (IsValid(Props.SourceASC) && Props.SourceASC->AbilityActorInfo.IsValid() && Props.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		// 获取施法者的AvatarActor（角色或物体）引用
		Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get();
		// 获取施法者的Controller
		Props.SourceController = Props.SourceASC->AbilityActorInfo->PlayerController.Get();
		// 如果控制器为空，并且AvatarActor有效，则从AvatarActor获取控制器
		if (Props.SourceController == nullptr && Props.SourceAvatarActor!=nullptr)
		{
			if(const APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				// 获取Pawn的控制器
				Props.SourceController = Pawn->GetController();
			}
		}
		// 如果控制器有效，从控制器获取角色信息
		if(Props.SourceController)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}
	}
	// 检查目标的AbilityActorInfo是否有效
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		// 获取目标Actor（受到效果的角色或物体）
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		// 获取目标的Controller
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		// 将目标Actor转换为角色
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		// 获取目标Actor的Ability System Component
		Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}
}



void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	//处理属性在网络上变化后的通知逻辑。
	//GAMEPLAYATTRIBUTE_REPNOTIFY宏用于处理属性更新，确保任何需要更新的引用（如UI）能正确响应属性的变化。
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,Health,OldHealth);
}


void UAuraAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,Strength,OldStrength);
}


// 当属性 Intelligence 在客户端同步时触发的函数
// 参数 OldIntelligence 表示属性更新前的旧值
void UAuraAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	// 调用 GAS 提供的宏 GAMEPLAYATTRIBUTE_REPNOTIFY
	// 自动处理属性更新后的通知逻辑（如触发回调或广播更新）
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,Intelligence,OldIntelligence);
}

void UAuraAttributeSet::OnRep_Resilience(const FGameplayAttributeData& OldResilience) const
{
	// 调用 GAS 提供的宏 GAMEPLAYATTRIBUTE_REPNOTIFY
	// 自动处理属性更新后的通知逻辑（如触发回调或广播更新）
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,Intelligence,OldResilience);
}

void UAuraAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	//处理属性在网络上变化后的通知逻辑。
	//GAMEPLAYATTRIBUTE_REPNOTIFY 宏用于处理属性更新，确保任何需要更新的引用（如UI）能正确响应属性的变化。
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,MaxHealth,OldVigor);
}

void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	//处理属性在网络上变化后的通知逻辑。
	//GAMEPLAYATTRIBUTE_REPNOTIFY 宏用于处理属性更新，确保任何需要更新的引用（如UI）能正确响应属性的变化。
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,MaxHealth,OldMaxHealth);
}

void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	//处理属性在网络上变化后的通知逻辑。
	//GAMEPLAYATTRIBUTE_REPNOTIFY 宏用于处理属性更新，确保任何需要更新的引用（如UI）能正确响应属性的变化。
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,Mana,OldMana);
}

void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	//处理属性在网络上变化后的通知逻辑。
    //GAMEPLAYATTRIBUTE_REPNOTIFY 宏用于处理属性更新，确保任何需要更新的引用（如UI）能正确响应属性的变化。
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,MaxMana,OldMaxMana);
}

void UAuraAttributeSet::OnRep_FireResistance(const FGameplayAttributeData& OldFireResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,FireResistance,OldFireResistance);
}

void UAuraAttributeSet::OnRep_LightningResistance(const FGameplayAttributeData& OldLightningResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,LightningResistance,OldLightningResistance);
}

void UAuraAttributeSet::OnRep_ArcaneResistance(const FGameplayAttributeData& OldArcaneResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,ArcaneResistance,OldArcaneResistance);
}

void UAuraAttributeSet::OnRep_PhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,PhysicalResistance,OldPhysicalResistance);
}


