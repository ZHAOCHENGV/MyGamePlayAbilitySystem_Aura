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

void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	
	Super::PreAttributeChange(Attribute, NewValue);
	//如果Attribute值 == 血量
	if (Attribute == GetHealthAttribute())
	{
		//这里的限制是针对查询（NewValue）的值，而不是属性的最终应用值。如果游戏逻辑中有多个地方在修改属性值，而没有使用 PreAttributeChange，则可能导致不一致
		// 限制健康值在 0 和最大健康值之间
		// 注意：此处只是在查询时限制值范围，实际应用数值可能在别处修改
		NewValue = FMath::Clamp(NewValue,0,GetMaxHealth());
		UE_LOG(LogTemp,Warning,TEXT("Health : %f"),NewValue);
	}
	
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue,0,GetMaxMana());
		UE_LOG(LogTemp,Warning,TEXT("Mana : %f"),NewValue);
	}
	
}

void UAuraAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	//创建数据结构
	FEffectProperties Props;
	//设置效果属性
	SetEffectProperties(Data,Props);
	// 数值应用时，限制 "Health" 属性的实际值在 0 和最大健康值之间
	// 此处直接对 SetHealth 和 SetMana 的结果进行限制，确保最终的属性值在有效范围内。
	// 这里是在实际应用效果后强制修正属性值，能确保逻辑一致性。
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(),0,GetMaxHealth()));
		UE_LOG(LogTemp,Warning,TEXT(" %s 的剩余血量为 : %f"), *Props.TargetAvatarActor->GetName(), GetHealth());
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(),0,GetMaxMana()));
	}
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		HandleIncomingDamage(Props);
	}
	// 当检测到经验值获取属性变化时的处理逻辑
	if (Data.EvaluatedData.Attribute == GetIncomingXPAttribute())
	{
		HandleIncomingXP(Props);
	}
}


void UAuraAttributeSet::HandleIncomingDamage(const FEffectProperties& Props)
{
	// 获取当前受到的伤害值
	const float LocalIncomingDamage = GetIncomingDamage();
	// 清空 "IncomingDamage"，表示这次伤害已经被处理
	SetIncomingDamage(0.f);
	if (LocalIncomingDamage > 0)
	{
		// 计算新的健康值（当前生命值减去伤害值）
		const float NewHealth = GetHealth() - LocalIncomingDamage;
		// 更新健康值，并确保健康值限制在 0 到最大健康值之间
		SetHealth(FMath::Clamp(NewHealth,0,GetMaxHealth()));
		// 判断伤害是否致命（健康值是否小于等于 0）
		const bool bFatal = NewHealth <= 0.f;
		if (bFatal)//死亡
		{
			ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor);
			if (CombatInterface)
			{
				CombatInterface->Die();
			}
				
			SendXPEvent(Props);
		}
		else//不死亡
		{
			// 创建一个标签容器用于标识 HitReact（受击反应）
			FGameplayTagContainer TagContainer;
			TagContainer.AddTag(FAuraGamePlayTags::Get().Effects_HitReact);
			// 尝试通过指定标签激活目标的能力
			Props.TargetASC->TryActivateAbilitiesByTag(TagContainer);
		}

		const bool bBlock = UAuraAbilitySystemLibrary::IsBlockedHit(Props.EffectContextHandle);
		const bool bCriticalHit = UAuraAbilitySystemLibrary::IsCriticalHit(Props.EffectContextHandle);
		ShowFloatingText(Props, LocalIncomingDamage, bBlock, bCriticalHit);
		if (UAuraAbilitySystemLibrary::IsSuccessfulDebuff(Props.EffectContextHandle))
		{
			Debuff(Props);
		}
	}
}

void UAuraAttributeSet::HandleIncomingXP(const FEffectProperties& Props)
{
	// 获取当前累积的待处理经验值（临时存储值）
	const float LocalIncomingXP = GetIncomingXP();
	// 立即清零经验获取属性（重要！防止重复处理）
	SetIncomingXP(0.f);
	// 输出日志
	UE_LOG(LogAura,Log,TEXT(" Incoming XP : %f"),LocalIncomingXP );

		
	// 处理经验获取和升级逻辑的核心函数
	if (Props.SourceCharacter->Implements<UPlayerInterface>() && Props.SourceCharacter->Implements<UCombatInterface>())
	{
		// 通过战斗接口获取当前角色等级
		const int32 CurrentLevel = ICombatInterface::Execute_GetPlayerLevel(Props.SourceCharacter);
		// 通过玩家接口获取当前经验值
		const int32 CurrentXP = IPlayerInterface::Execute_GetXP(Props.SourceCharacter);
		// 计算增加经验后的新等级
		const int32 NewLevel = IPlayerInterface::Execute_FindLevelForXP(Props.SourceCharacter, CurrentXP + LocalIncomingXP);
		// 计算升级次数
		const int32 NumLevelUps = NewLevel - CurrentLevel;
		//如果升级次数大于0，则执行升级逻辑
		if (NumLevelUps > 0)
		{
			const int32 AttributePointsReward = IPlayerInterface::Execute_GetAttributePointsReward(Props.SourceCharacter, CurrentLevel);
			const int32 SpellPointsReward = IPlayerInterface::Execute_GetSpellPointsReward(Props.SourceCharacter, CurrentLevel);

			IPlayerInterface::Execute_AddToPlayerLevel(Props.SourceCharacter,NumLevelUps);
			IPlayerInterface::Execute_AddToAttributePoints(Props.SourceCharacter,AttributePointsReward);
			IPlayerInterface::Execute_AddToSpellPoints(Props.SourceCharacter,SpellPointsReward);

			//设置到达最大生命值和最大魔力值
			bTopOffHealth = true;
			bTopOffMana = true;
				
				
				

			// 执行升级操作（应循环处理多次升级）
			IPlayerInterface::Execute_LeveUp(Props.SourceCharacter);
		}
		// 添加经验值（需确保在服务端执行）
		IPlayerInterface::Execute_AddToXP(Props.SourceCharacter, LocalIncomingXP);
	}
}

/**
 * @brief 根据 EffectContext 中的 Debuff 参数，临时构造一个“周期伤害（DoT）”类 UGameplayEffect 并施加到目标
 * @param Props 作用参数集合：包含 SourceASC/TargetASC、SourceAvatarActor、EffectContextHandle 等
 * @return void  无返回；副作用是对 TargetASC 施加一个按频率跳伤的临时 GE
 *
 * 功能说明：
 * - 从 Props.EffectContextHandle 读取 Debuff 相关参数（DamageType、DebuffDamage、DebuffDuration、DebuffFrequency）。
 * - 运行时用 NewObject 动态创建一个 UGameplayEffect，配置“有持续时间 + 周期触发”，并添加对 IncomingDamage 的修饰器。
 * - 构建 FGameplayEffectSpec，补写 DamageType 到自定义 Context，随后对 TargetASC 执行 ApplyGameplayEffectSpecToSelf。
 *
 * 背景知识（UE/GAS）：
 * - UGameplayEffect 是“配方”，FGameplayEffectSpec 是“实例单”；Period>0 表示按固定间隔重复生效（DoT）。
 * - EffectContext 记录施加者/来源对象/命中等信息；我们扩展的 Context 还带有 DamageType 等自定义字段。
 * - Attribute 修饰器（Modifiers）描述对某个 Attribute 的变更：加法/乘法/覆盖等。
 *
 * 详细流程：
 * 1) 取标签单例与用 SourceASC 构建 EffectContext，并记录 SourceAvatarActor 作为 SourceObject。
 * 2) 从上游 Props.EffectContextHandle 读出 DamageType、DebuffDamage、DebuffDuration、DebuffFrequency。
 * 3) New 一个临时 UGameplayEffect：设置 DurationPolicy=HasDuration、Period=DebuffFrequency、Duration=DebuffDuration，
 *    并贴上对应的 Debuff Tag，设置叠加策略（同来源聚合，最大 1 层）。
 * 4) 在 GE 上添加一条对 IncomingDamage 的 Additive 修饰器，幅度使用 DebuffDamage（每跳伤害）。
 * 5) 直接 new 一个 FGameplayEffectSpec（使用上面构造的 Effect 与 EffectContext），把 DamageType 写回 Context，
 *    然后调用 TargetASC->ApplyGameplayEffectSpecToSelf 应用。
 *
 * 注意事项（新手易错）：
 * - GamePlayTags.DamageTypesDebuffs[DamageType] 使用前应确保映射存在该键；否则可能访问越界（开发期可 ensure）。
 * - DebuffFrequency 应 > 0，否则 Period=0 将导致周期效果无效或异常；DebuffDuration 应 >= 0。
 * - 直接 new FGameplayEffectSpec 存在生命周期管理风险；常见做法是使用 FGameplayEffectSpecHandle 或栈对象。
 * - NewObject(GetTransientPackage(), ...) 返回瞬时对象；仅本次使用，避免长期持有导致 GC 相关问题。
 */
void UAuraAttributeSet::Debuff(const FEffectProperties& Props)
{
	// 步骤 1：拿到项目统一的 GameplayTags（包含各种伤害/减益的 Tag 映射）
	const FAuraGamePlayTags& GamePlayTags = FAuraGamePlayTags::Get();
	// 步骤 2：用 SourceASC 创建一个 EffectContext（描述这次效果的“处方上下文”）
	FGameplayEffectContextHandle EffectContext= Props.SourceASC->MakeEffectContext();
	// 步骤 3：把 SourceAvatarActor 记入 Context 的 SourceObject（便于溯源、Cue 展示）
	EffectContext.AddSourceObject(Props.SourceAvatarActor);

	// 步骤 4：从上游的 EffectContextHandle 读取伤害类型（FGameplayTag）
	const FGameplayTag DamageType = UAuraAbilitySystemLibrary::GetDamageType(Props.EffectContextHandle);
	// 步骤 5：读取每跳伤害（float）
	const float DebuffDamage = UAuraAbilitySystemLibrary::GetDebuffDamage(Props.EffectContextHandle);
	// 步骤 6：读取总持续时间（秒）
	const float DebuffDuration = UAuraAbilitySystemLibrary::GetDebuffDuration(Props.EffectContextHandle);
	// 步骤 7：读取跳伤频率（间隔秒数，Period）
	const float DebuffFrequency = UAuraAbilitySystemLibrary::GetDebuffFrequency(Props.EffectContextHandle);

	// 步骤 8：拼一个易读的临时 GE 名称（带上伤害类型，方便日志/调试）
	FString DebuffName = FString::Printf(TEXT("DynamicDebuff_%s"),*DamageType.ToString());
	// 步骤 9：在瞬时包中动态创建一个 UGameplayEffect（仅用于本次应用）
	UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(DebuffName));

	// 步骤 10：设置“有持续时间”的策略（HasDuration）
	Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
	// 步骤 11：设置周期触发间隔（秒）；Period>0 才会按间隔触发
	Effect->Period = DebuffFrequency;
	// 步骤 12：设置持续时间（可伸缩浮点封装）
	Effect->DurationMagnitude = FScalableFloat(DebuffDuration);
	// 步骤 13：给 GE 添加“本次减益类型”的 Tag（用于过滤/查询/表现）
	Effect->InheritableOwnedTagsContainer.AddTag(GamePlayTags.DamageTypesDebuffs[DamageType]);
	// 步骤 14：设置叠加策略：同一来源聚合
	Effect->StackingType = EGameplayEffectStackingType::AggregateBySource;
	// 步骤 15：限制最大叠加层数为 1（防止多层无限叠加）
	Effect->StackLimitCount = 1;

	// 步骤 16：准备在 GE 上新增一个修饰器（Modifier），影响 IncomingDamage
	const int32 Index = Effect->Modifiers.Num();                           // 记录当前尾索引
	Effect->Modifiers.Add(FGameplayModifierInfo());                        // 在尾部追加一个默认修饰器
	FGameplayModifierInfo& ModifierInfo = Effect->Modifiers[Index];        // 引用该新修饰器
	
	// 步骤 17：把每跳伤害 DebuffDamage 作为修饰器的幅度（可被曲线/等级缩放）
	ModifierInfo.ModifierMagnitude = FScalableFloat(DebuffDamage);
	// 步骤 18：修改方式为“加法叠加”（EGameplayModOp::Additive）
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	// 步骤 19：目标属性为 IncomingDamage（通常用于汇总即将结算的伤害）
	ModifierInfo.Attribute = UAuraAttributeSet::GetIncomingDamageAttribute();

	// 步骤 20：构造一个 FGameplayEffectSpec（实例单），并把 DamageType 回写到自定义 Context
	if (FGameplayEffectSpec* MutableSpec = new FGameplayEffectSpec(Effect, EffectContext, 1.f)) // 直接 new 一个 Spec（等级 1.f）
	{
		// 步骤 21：从 Spec 获取我们扩展的 FAuraGameplayEffectContext 指针（用于写入自定义字段）
		FAuraGameplayEffectContext* AuraContext = static_cast<FAuraGameplayEffectContext*>(MutableSpec->GetContext().Get());
		// 步骤 22：为 DamageType 构造一个共享指针（与 Context 的字段类型匹配）
		TSharedPtr<FGameplayTag> DebuffDamageType = MakeShareable(new FGameplayTag(DamageType));
		// 步骤 23：把 DamageType 写回 Context（保证下游读取一致）
		AuraContext->SetDamageType(DebuffDamageType);
		// 步骤 24：对 TargetASC 施加这个临时构造的 Spec（开始按周期造成伤害）
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


