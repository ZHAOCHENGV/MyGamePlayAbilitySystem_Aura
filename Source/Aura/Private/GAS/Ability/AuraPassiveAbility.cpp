// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/AuraPassiveAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/AuraAbilitySystemComponent.h"

/**
 * @brief 被动技能（GA）激活时的回调：在 ASC 上绑定“被动停用”事件，以便外部下发停用时结束本 GA
 *
 * @param Handle             本次激活的能力句柄
 * @param ActorInfo          能力执行体信息（Owner/Avatar/PC 等）
 * @param ActivationInfo     激活信息（本地/远端、预测等）
 * @param TriggerEventData   触发数据（事件型激活时可携带）
 *
 * 功能说明：
 * - 调用父类激活逻辑；
 * - 从 Avatar 上获取 AuraASC，绑定其 `DeactivatePassiveAbility` 委托到本类 `ReceiveDeactivate`；
 * - 当 ASC 广播“某被动能力需要停用”时，`ReceiveDeactivate` 会比对标签并结束本 GA。
 *
 * 详细流程：
 * 1) Super::ActivateAbility → 保持 GA 基类行为；
 * 2) 通过 UAbilitySystemBlueprintLibrary 从 Avatar 取 ASC，并强转 UAuraAbilitySystemComponent；
 * 3) 绑定 ASC 的 DeactivatePassiveAbility（Broadcast 的源）到本类回调。
 *
 * 注意事项：
 * - 建议本 GA 的 InstancingPolicy=InstancedPerActor，确保“每个拥有者一个实例”，避免共享回调串线；
 * - 多次激活前最好避免重复绑定（见下方“建议与优化”）；销毁/结束时请解绑，防止悬空回调；
 * - 被动 GA 一般不预测执行，NetExecutionPolicy 需与项目一致（ServerOnly/ServerInitiated/LocalOnly 等）。
 */
void UAuraPassiveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
										  const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
										  const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData); // 调父类逻辑（成本/冷却/标签等）

	// 从 Avatar 获取 ASC，并强转为自定义 ASC 类型（用于访问自定义委托）
	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo())))
	{
		// 把 ASC 的“被动停用”广播事件，绑定到本类的 ReceiveDeactivate 回调
		AuraASC->DeactivatePassiveAbility.AddUObject(this, &UAuraPassiveAbility::ReceiveDeactivate); // 绑定成员函数
	}
}

/**
 * @brief ASC 广播“被动停用”时的接收回调：若标签匹配当前 GA，就结束本能力
 *
 * @param AbilityTag 需要被停用/结束的“被动能力标签”
 *
 * 功能说明：
 * - 比对传入标签是否与本 GA 的 AbilityTags 精确匹配；
 * - 若匹配，则调用 EndAbility 结束本次被动（并选择复制到远端、标记为取消）。
 *
 * 详细流程：
 * 1) 使用 AbilityTags.HasTagExact 做精确匹配；
 * 2) 若命中 → EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, ReplicateEnd = true, WasCancelled = true)。
 *
 * 注意事项：
 * - EndAbility 的最后两个布尔参数分别表示“是否复制结束到远端”“本次是否作为取消处理”，与 UI/逻辑联动有关；
 * - 若你的项目希望区分“自然结束”与“被动被关闭”，可以根据需要把 WasCancelled 置为 false 并在外部做区分。
 */
void UAuraPassiveAbility::ReceiveDeactivate(const FGameplayTag& AbilityTag)
{
	// 仅在标签与本 GA 的标签精确匹配时才响应（避免误终止其他被动）
	if (AbilityTags.HasTagExact(AbilityTag))
	{
		// 结束本 GA：复制结束到远端，同时标记为“取消结束”（符合“被动被关闭”的语义）
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}
