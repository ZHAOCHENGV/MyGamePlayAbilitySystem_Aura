// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Passive/PassiveNiagaraComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGamePlayTags.h"
#include "GAS/AuraAbilitySystemComponent.h"
#include "Interation/CombatInterface.h"

UPassiveNiagaraComponent::UPassiveNiagaraComponent()
{
	bAutoActivate = false;
}

/**
 * @brief 组件开始时绑定“被动启停”事件（来自 ASC），以控制本 Niagara 特效的激活/关闭
 *
 * 功能说明：
 * - 优先直接从 Owner 取 ASC 并绑定 `ActivatePassiveEffect` → `OnPassiveActivate`；
 * - 若此时 ASC 尚未注册，则从 CombatInterface 的 “ASCRegistered” 委托里再绑定一次（迟到绑定）。
 *
 * 详细流程：
 * 1) Super::BeginPlay 调父类；
 * 2) 直接从 Owner 获取 ASC，若成功 → 绑定委托；
 * 3) 否则判断是否实现 CombatInterface，若是 → 监听 “ASC 注册完成” 并在回调里完成绑定。
 *
 * 注意事项：
 * - 这里使用 AddUObject 绑定，若可能多次绑定，建议先 RemoveAll(this) 再 Add（见文末建议）；
 * - Lambda 捕获 this 属于强引用，建议改用 AddWeakLambda（见文末建议）；
 * - 在回调里应优先使用回调参数 ASC，而不是再次从 Owner 查询（见文末建议）。
 */
void UPassiveNiagaraComponent::BeginPlay() 
{
	Super::BeginPlay(); // 调用父类 BeginPlay

	// 方式一：尝试直接从 Owner 获取 ASC（若已存在）
	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))) // 从 Owner 拿 ASC
	{
		AuraASC->ActivatePassiveEffect.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveActivate); // 绑定：ASC 广播 → 本组件回调
		ActivateIfEquipped(AuraASC);
	}
	// 方式二：此时还没有 ASC，则等 ASC 注册后再绑定
	else if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner())) // Owner 实现了 CombatInterface？
	{
		CombatInterface->GetOnASCRegisteredDelegate().AddLambda([this](UAbilitySystemComponent* ASC) // 监听“ASC 已注册”
		{
			if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))) // 再取 ASC
			{
				AuraASC->ActivatePassiveEffect.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveActivate); // 完成绑定
				ActivateIfEquipped(AuraASC);
			}
			
		});
	}
}

/**
 * @brief 被动启停事件回调：匹配到本组件的 PassiveSpellTag 后，按 bActivate 控制 Niagara 的激活/关闭
 *
 * @param AbilityTag  触发的被动能力标签（需与本组件 PassiveSpellTag 精确匹配）
 * @param bActivate   true=应当激活，false=应当关闭
 *
 * 功能说明：
 * - 只处理“标签精确匹配”的事件，避免误触其他被动；
 * - 当 bActivate 且当前未激活 → Activate()；否则执行 Deactivate()。
 *
 * 注意事项：
 * - 现有写法是“只要不满足（bActivate && !IsActive）就 Deactivate”，
 *   若在已激活状态下再次收到 bActivate=true，会被 Deactivate（潜在逻辑坑，见文末建议的“状态对齐”写法）。
 */
void UPassiveNiagaraComponent::OnPassiveActivate(const FGameplayTag& AbilityTag, bool bActivate)
{
	if (AbilityTag.MatchesTagExact(PassiveSpellTag))              // 仅处理与本组件配置的 PassiveSpellTag 精确匹配的事件
	{
		if (bActivate && !IsActive())                             // 需要激活，且当前未激活
		{
			Activate();                                           // 激活 Niagara
		}
		else                                                      // 其他情况（包含 bActivate=false，或已激活但重复收到 bActivate=true）
		{
			Deactivate();                                         // 统一关闭（注意：会导致“重复激活事件”把已激活状态关掉，见建议）
		}
	}
}




/**
 * @brief 检查一个关联的被动技能是否已被“装备”，如果是，则激活此 Niagara 组件（播放特效）。
 * @param AuraASC 需要查询其状态的 AuraAbilitySystemComponent。
 *
 * @par 功能说明
 * 该函数是连接一个被动技能（Passive Ability）的状态和其视觉表现（Niagara 特效）的纽タ。
 * 这个 Niagara 组件本身代表了一个被动效果（比如一个光环或一个持续的护盾特效）。
 * 此函数被调用时，它会去查询传入的 ASC，看看与这个特效所绑定的那个被动技能（由 `PassiveSpellTag` 指定）
 * 当前是否处于“已装备”状态。如果是，它就激活自己，让特效播放出来。
 *
 * @par 详细流程
 * 1.  **初始化检查**: 首先检查 ASC 的 `bStartupAbilitiesGiven` 标志。这是一个非常重要的安全检查，确保在查询技能状态之前，ASC 已经完成了它的初始化流程并授予了所有初始技能。这可以防止在错误的时机（过早）进行查询而导致逻辑错误。
 * 2.  **状态查询**: 如果 ASC 已准备就绪，则调用一个自定义的辅助函数 `AuraASC->GetStatusFromAbilityTag()`，并传入本组件上配置的 `PassiveSpellTag`。这个函数的作用是根据技能标签查找到对应的技能，并返回该技能的当前状态（很可能是另一个 Gameplay Tag）。
 * 3.  **状态比较**: 将查询返回的状态 Tag 与全局定义的“已装备”状态 Tag (`FAuraGamePlayTags::Get().Abilities_Status_Equipped`) 进行比较。
 * 4.  **激活组件**: 如果两个 Tag 完全匹配，说明该被动技能确实处于“已装备”状态，于是调用 `Activate()` 函数。对于 Niagara 组件来说，`Activate()` 会启动粒子特效的播放。
 *
 * @par 注意事项
 * - `GetStatusFromAbilityTag` 看起来是一个项目自定义的 C++ 函数，而不是 UE GAS 的标准函数。它的实现细节决定了状态查询的准确性。
 * - 此函数是一个“一次性”的检查。它只在被调用的那个时间点检查状态。如果技能是在此函数调用之后才被装备的，特效将不会自动激活。一个更健壮的系统通常会使用委托（Delegate）来监听状态变化事件。
 */
void UPassiveNiagaraComponent::ActivateIfEquipped(UAuraAbilitySystemComponent* AuraASC)
{
	// 步骤 1/3: 安全检查，确保 ASC 已经完成了初始技能的授予流程。
	// (为什么这么做): 这是一个防止竞争条件 (Race Condition) 的关键检查。如果没有这个，
	// 此函数若在 ASC 初始化完成前被调用，查询到的技能状态将是不完整的或错误的。
	if (AuraASC->bStartupAbilitiesGiven)
	{
		// 步骤 2/3: 查询与本组件关联的被动技能的当前状态。
		// PassiveSpellTag 是本组件的一个 UPROPERTY，在蓝图中设置，用于将这个特效与一个具体的技能Tag绑定。
		// GetStatusFromAbilityTag 是一个自定义函数，它根据技能Tag返回一个代表其状态的Tag（例如：已装备、未装备、冷却中等）。
		// FAuraGamePlayTags::Get().Abilities_Status_Equipped 是一个全局单例，用于获取“已装备”这个状态的 Gameplay Tag。
		if (AuraASC->GetStatusFromAbilityTag(PassiveSpellTag) == FAuraGamePlayTags::Get().Abilities_Status_Equipped)
		{
			// 步骤 3/3: 如果状态匹配，则激活本组件。
			// Activate() 是 UActorComponent 的标准函数。对 Niagara 组件而言，它会开始播放粒子特效。
			Activate();
		}
	}
}
