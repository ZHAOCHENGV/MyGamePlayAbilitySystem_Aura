// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Passive/PassiveNiagaraComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
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
	}
	// 方式二：此时还没有 ASC，则等 ASC 注册后再绑定
	else if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner())) // Owner 实现了 CombatInterface？
	{
		CombatInterface->GetOnASCRegisteredDelegate().AddLambda([this](UAbilitySystemComponent* ASC) // 监听“ASC 已注册”
		{
			if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))) // 再取 ASC
			{
				AuraASC->ActivatePassiveEffect.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveActivate); // 完成绑定
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
