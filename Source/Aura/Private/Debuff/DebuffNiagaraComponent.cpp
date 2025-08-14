// Fill out your copyright notice in the Description page of Project Settings.


#include "Debuff/DebuffNiagaraComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interation/CombatInterface.h"

UDebuffNiagaraComponent::UDebuffNiagaraComponent()
{
	bAutoActivate = false;
}

/**
 * @brief 监听并响应 Owner 身上的 Debuff Tag 改变（开/关 Niagara 特效），以及在死亡时关闭特效
 * @details
 *  - 【作用】在组件 BeginPlay 时，尝试获取 Owner 的 AbilitySystemComponent（ASC）；若已存在，直接注册 GameplayTagEvent；
 *    若尚未存在但 Owner 实现了 ICombatInterface，则监听“ASC 注册完成”的委托后再注册 Tag 事件；同时监听“死亡”事件以关闭特效。
 *  - 【背景知识】GAS 的 GameplayTagEvent 可在标签数发生变化时回调（NewOrRemoved），常用于 UI/特效同步；ICombatInterface
 *    是项目内约定的接口，暴露 ASC 注册/死亡等生命周期委托；NiagaraComponent 的 Activate/Deactivate 控制特效开关。
 *  - 【详细流程（BeginPlay）】
 *    1) 调用父类 BeginPlay；2) 从 Owner 转成 ICombatInterface；3) 直接尝试从 Owner 取 ASC → 成功：注册 DebuffTag 事件；
 *       4) 若失败且有 CombatInterface：订阅 OnASCRegistered，再在回调内注册 DebuffTag 事件；5) 若有 CombatInterface：订阅 OnDeath 关闭特效。
 *  - 【注意事项】
 *    - RegisterGameplayTagEvent 返回的 FDelegateHandle 建议保存，以便在 EndPlay 里解绑，避免悬挂回调。
 *    - DebuffTag 应为有效的 GameplayTag；事件类型用 NewOrRemoved 表示从 0→1 或 1→0 等边沿变化。
 */
void UDebuffNiagaraComponent::BeginPlay()
{
	// 步骤 1：先调用父类 BeginPlay，保持组件正常初始化链
	Super::BeginPlay();
	// 步骤 2：尝试将 Owner 转成 ICombatInterface（便于订阅 ASCRegistered/Death 等自定义事件）
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner());
	// 步骤 3：直接尝试从 Owner 拿 ASC；若拿到则立即注册 DebuffTag 的 NewOrRemoved 事件
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		// 步骤 3.1：ASC 已存在 → 直接注册 Tag 事件，Tag 计数变化时回调 DebuffTagChanged
		ASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this,&UDebuffNiagaraComponent::DebuffTagChanged);
	}
	// 步骤 4：若此时还拿不到 ASC，但 Owner 实现了 ICombatInterface，则等 ASC 注册完成再绑定
	else if (CombatInterface)
	{
		// 步骤 4.1：订阅“ASC 注册完成”委托；使用弱捕获避免悬挂；回调参数是新注册的 InASC
		CombatInterface->GetOnASCRegisteredDelegate().AddLambda([this](UAbilitySystemComponent* NewASC)
		{
			// 步骤 4.2：ASC 刚注册好 → 现在再注册 DebuffTag 事件
			NewASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this,&UDebuffNiagaraComponent::DebuffTagChanged);;
		});
	}
	// 步骤 5：如果有 CombatInterface，则订阅“死亡事件”，在死亡时关闭特效
	if (CombatInterface)
	{
		// 步骤 5.1：绑定 OnDeath 到本组件的 OnOwnerDeath（动态绑定，便于蓝图/反射）
		CombatInterface->GetOnDeathDelegate().AddDynamic(this, &UDebuffNiagaraComponent::OnOwnerDeath);
	}
}
/**
 * @brief Debuff Tag 计数变化时的回调：大于 0 激活特效；等于 0 关闭特效
 * @param CallbackTag 哪个 Tag 触发（此处就是 DebuffTag）
 * @param NewCount    标签计数的新值（>0 表示拥有该 Tag）
 * @details GameplayTagEvent(NewOrRemoved) 在计数从 0↔1 时触发，适合做开关型表现
 */
void UDebuffNiagaraComponent::DebuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	// 步骤 1：如果标签计数大于 0，说明处于“有 Debuff”状态 → 打开 Niagara
	if (NewCount > 0)
	{
		Activate(); // 开特效
	}
	// 步骤 2：否则关闭特效
	else
	{
		Deactivate(); // 关特效
	}
}

/**
 * @brief Owner 死亡时关闭特效（兜底）
 * @param DeadActor 死亡的 Actor（通常就是 GetOwner()）
 */
void UDebuffNiagaraComponent::OnOwnerDeath(AActor* DeadActor)
{
	// 步骤 1：无条件关闭特效（防止残留）
	Deactivate(); // 关特效
}

