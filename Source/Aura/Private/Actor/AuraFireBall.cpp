// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraFireBall.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGamePlayTags.h"
#include "Components/AudioComponent.h"
#include "GameplayCueManager.h"
#include "GAS/AuraAbilitySystemLibrary.h"

void AAuraFireBall::BeginPlay()
{
	Super::BeginPlay();
	StartOutgoingTimeline();
}

/**
 * @brief 处理火球与其它 Actor 发生重叠时的核心逻辑，主要负责在服务器上施加伤害。
 *
 * @param OverlappedComponent 触发这次重叠的本 Actor（火球）的组件。
 * @param OtherActor 与火球发生重叠的另一个 Actor。
 * @param OtherComp 另一个 Actor 中与火球发生重叠的组件。
 * @param OtherBodyIndex 另一个 Actor 组件的物理形体索引（常用于复杂的物理模拟）。
 * @param bFromSweep 是否由扫描（Sweep）过程触发的重叠。
 * @param SweepResult 本次扫描的命中结果详情。
 *
 * @par 功能说明
 * 该函数是火球造成伤害的关键入口。当火球的碰撞体（如 USphereComponent）与其它物体发生重叠时，
 * Unreal Engine 会自动调用这个函数。它的核心职责是验证目标，并仅在服务器上应用伤害效果。
 *
 * @par 详细流程
 * 1.  **目标有效性检查**：调用 `IsValidOverlap(OtherActor)` 函数，过滤掉无效的碰撞目标，例如火球的发射者自身、友方单位或已经死亡的单位。这是避免误伤和无效计算的第一道防线。
 * 2.  **服务器权限验证**：使用 `HasAuthority()` 检查，确保接下来的伤害逻辑只在服务器上执行。这是网络游戏中的黄金法则，防止客户端作弊或逻辑不一致。
 * 3.  **获取目标 ASC**：尝试从 `OtherActor` 获取其 GAS 核心组件——`UAbilitySystemComponent` (ASC - 游戏能力系统组件)。如果目标没有 ASC，则无法对其施加基于 GAS 的效果。
 * 4.  **准备伤害参数**：
 *     - 计算死亡时的冲击力向量（`DeathImpulse`），方向为火球的前进方向。
 *     - 将冲击力向量和目标 ASC 存入 `DamageEffectParams` 结构体中。这个结构体显然是一个自定义的、用于统一传递伤害相关参数的工具。
 * 5.  **应用伤害效果**：调用一个自定义的静态库函数 `UAuraAbilitySystemLibrary::ApplyDamageEffect`，传入准备好的参数，来应用一个或多个 Gameplay Effect (GE - 游戏效果)，最终对目标造成伤害。
 *
 * @par 注意事项
 * - 此函数严格遵循了 Server-Authoritative（服务器权威）模型，所有影响游戏状态的核心逻辑（如扣血）都由服务器说了算。
 * - 伤害的施加依赖于目标必须拥有 `UAbilitySystemComponent`。
 * - 具体的伤害数值、效果类型（如火焰伤害）等被封装在了 `DamageEffectParams` 和 `ApplyDamageEffect` 函数内部，使得此处的逻辑保持清晰，只负责“触发”这一行为。
 */
void AAuraFireBall::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
									AActor* OtherActor,
									UPrimitiveComponent* OtherComp,
									int32 OtherBodyIndex,
									bool bFromSweep,
									const FHitResult& SweepResult)
{
	// 步骤 1/3: 检查碰撞目标是否有效，例如，不能是自己，也不能是友军。如果无效，则立即中断函数。
	if (!IsValidOverlap(OtherActor)) return;

	// 步骤 2/3: 检查当前代码是否运行在服务器上。HasAuthority() 在服务器上返回 true，在客户端返回 false。
	// (为什么这么做): 这是为了保证伤害计算等核心游戏逻辑只在服务器上执行一次，防止客户端作弊，并确保所有玩家的游戏状态一致。
	if (HasAuthority())
	{
		// 尝试从被命中的 Actor (OtherActor) 身上获取它的 AbilitySystemComponent (ASC - 游戏能力系统组件)。
		// ASC 是 GAS 框架的核心，管理着一个 Actor 的所有技能、属性和状态效果。
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			// 步骤 3/3: 准备并应用伤害效果
			// 计算一个死亡冲击力，方向是火球飞行的前向，大小由 DamageEffectParams.DeathImpulseMagnitude 决定。
			const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;

			// 将计算出的冲击力向量赋值给参数结构体。
			DamageEffectParams.DeathImpulse = DeathImpulse;
			// 将目标 Actor 的 ASC 实例也存入参数结构体，告知伤害该施加给谁。
			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;

			// 调用一个自定义的静态库函数来应用伤害。这是一种良好的封装，将伤害处理的通用逻辑（如创建 GE 并应用）放到一个可复用的地
			// Gameplay Effect (GE - 游戏效果) 是 GAS 中定义数据变化（如扣血、减速）的模板资源。
			UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}
	}
}

/**
 * @brief 在火球命中目标或障碍物时调用，负责处理非 gameplay 核心的命中表现，如特效和音效。
 *
 * @par 功能说明
 * 这个函数用于触发命中时的即时反馈，例如爆炸的视觉效果（VFX）和音效（SFX）。它还负责清理火球自身的一些资源，如循环播放的飞行音效。
 *
* @par 详细流程
 * 1.  **触发 GameplayCue**：
 *     - 检查火球的 `Owner`（发射者）是否存在。
 *     - 如果存在，则在火球当前的位置触发一个 GameplayCue (GC - 游戏逻辑提示)。GC 是 GAS 中专门用于处理装饰性、非关键逻辑（如粒子特效、音效）的系统。
 *     - 这里使用了 `ExecuteGameplayCue_NonReplicated`，意味着这个 GC 只会在调用它的机器上本地播放，不会自动同步到其他客户端。
 * 2.  **清理循环音效**：检查是否存在 `LoopingSoundComponent`（火球飞行时的持续音效），如果存在，则停止播放并销毁该组件，防止声音在火球消失后继续存在。
 * 3.  **设置命中状态**：将布尔变量 `bHit` 设为 `true`。这是一个状态锁，用于防止例如在穿透多个目标或快速连续重叠时，重复执行命中逻辑。
 *
 * @par 注意事项
 * - `ExecuteGameplayCue_NonReplicated` 的使用需要特别注意。如果 `OnHit()` 只在服务器上被调用，那么爆炸特效将只在服务器上播放，客户端是看不到的。通常需要配合一个 NetMulticast RPC 来在所有客户端上调用 `OnHit()`，或者使用一个配置为可复制的 GameplayCue Tag。
 * - 资源清理（如销毁声音组件）是一个好习惯，可以防止内存和性能泄漏。
 */
void AAuraFireBall::OnHit()
{
	// 步骤 1/3: 触发爆炸特效/音效
	// 创建一个 GameplayCue (GC - 游戏逻辑提示) 的参数对象。GC 主要用于触发非核心 gameplay 的表现，如粒子特效和声音。
	if (GetOwner())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = GetActorLocation();// 将火球的当前位置作为特效生成点。

		// 在火球的 Owner 上执行一个“非复制”的 GameplayCue。
		// (为什么这么做): `_NonReplicated` 意味着这个特效只会在调用此代码的机器上播放。
		// 如果 OnHit() 只在服务器上调用，客户端将看不到爆炸。通常需要配合网络复制来让所有人都看到。
		UGameplayCueManager::ExecuteGameplayCue_NonReplicated(
			GetOwner(),// GC 的发起者
			FAuraGamePlayTags::Get().GameplayCue_FireBlast,// 具体的 GC 标签，例如 "GameplayCue.FireBlast"
			CueParams// 传递位置等参数
		);
	}

	// 步骤 2/3: 清理飞行中的循环音效
	// 如果火球带有一个循环播放的音效组件（例如飞行的“咻咻”声）...
	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop(); // ...立即停止播放。
		LoopingSoundComponent->DestroyComponent();// ...并销毁该组件，释放资源。
	}

	// 步骤 3/3: 设置状态锁，防止重复触发
	// 将 bHit 标记为 true，这样即使之后再发生重叠事件，也可以通过检查这个变量来避免重复执行 OnHit 逻辑。
	bHit = true;
}
