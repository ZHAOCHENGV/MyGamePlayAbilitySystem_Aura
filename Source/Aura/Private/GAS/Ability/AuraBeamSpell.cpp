// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/AuraBeamSpell.h"

#include "GameFramework/Character.h"
#include "GAS/AuraAbilitySystemLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

/**
 * @brief 从鼠标射线命中的结果里缓存“光束目标”的坐标与目标Actor；未命中则取消本次技能
 * @param HitResult 鼠标/准星的射线检测结果（含 bBlockingHit、命中位置、命中Actor 等）
 *
 * 详细流程：
 * 1) 判断是否发生“阻挡命中”（bBlockingHit）。 
 * 2) 命中：记录 Location（坐标）与 GetActor（目标）。 
 * 3) 未命中：调用 CancelAbility 取消当前激活（并带复制，保持多人一致）。
 *
 * 注意事项：
 * - bBlockingHit 仅代表射线被碰撞体阻挡，不等同于“可攻击目标”；必要时继续做阵营/可选过滤。
 * - CancelAbility 的最后一个参数 true 表示将取消事件通过网络复制到服务器/其他客户端。
 */
void UAuraBeamSpell::StoreMouseDataInfo(const FHitResult& HitResult)
{
	// 是否发生了“阻挡命中”（射线被某个碰撞体拦下）
	if (HitResult.bBlockingHit)
	{
		// 记录命中世界坐标（后续用于光束起点→目标的方向/长度）
		MouseHitLocation = HitResult.Location;
		// 记录命中的Actor（用于锁定、朝向或后续验证）
		MouseHitActor = HitResult.GetActor();
	}
	else
	{
		// 未命中任何阻挡物：取消本次技能（四参true=进行网络复制）
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

/**
 * @brief 缓存“施法者”的常用指针（PlayerController / 角色Avatar），便于后续查询与权限判断
 * @details
 * - 从 CurrentActorInfo 中拿到 PlayerController 与 AvatarActor（转为 ACharacter）。
 * - 常用于：客户端光标检测、HUD交互、权限/Authority 判定等。
 *
 * 注意事项：
 * - AI 没有 PlayerController；多人下 PlayerController 只在本地拥有者有效。
 * - AvatarActor 可能不是 ACharacter（比如 Pawn 或其它 Actor），这里用 Cast 做安全转换。
 */
void UAuraBeamSpell::StoreOwnerVariables()
{
	// GAS 在激活阶段会填充 CurrentActorInfo，先判空再取
	if (CurrentActorInfo)
	{	
		// 缓存玩家控制器（客户端相关操作常用）
		OwnerPlayerController = CurrentActorInfo->PlayerController.Get();
		// 缓存AvatarActor并尝试转成 ACharacter（便于拿Mesh、移动等）
		OwnerCharacter = Cast<ACharacter>(CurrentActorInfo->AvatarActor);
	}
}

/**
 * @brief 用“武器尖端”到“目标位置”的小球体扫射（SphereTrace）来锁定首个光束目标
 * @param BeamTargetLocation 期望的光束目标位置（通常是鼠标命中的地面点/单位位置）
 *
 * 详细流程：
 * 1) 校验 OwnerCharacter 存在。 
 * 2) 若角色实现 ICombatInterface：取武器 SkeletalMesh 的 Socket（TipSocket）位置作为起点。 
 * 3) 构造忽略列表（至少忽略自己）。 
 * 4) 从起点 → 目标点做一次 SphereTraceSingle（半径10）。 
 * 5) 命中：更新 MouseHitLocation/MouseHitActor（以命中点为准）。 
 * 6) 未命中：保持原有鼠标信息（或按需做兜底）。
 *
 * 注意事项：
 * - TraceTypeQuery1 是“自定义Trace通道1”的别名，建议改为具名通道（如 TraceTypeQuery_Target）或使用自定义的 ECollisionChannel，避免魔法枚举。
 * - Socket 名称 "TipSocket" 要与武器资源一致；否则 GetSocketLocation 会返回(0,0,0)或失败。
 * - SphereTraceSingle 第9参 EDrawDebugTrace 可切换为 ForDuration 调试可视化命中。
 */
void UAuraBeamSpell::TraceFirstTarget(const FVector& BeamTargetLocation)
{
	// 运行时断言：OwnerCharacter 必须已缓存（若未设置将崩溃，便于早期暴露问题）
	check(OwnerCharacter);

	// 仅当角色实现了 ICombatInterface（约定可取武器）时才继续
	if (OwnerCharacter->Implements<UCombatInterface>())
	{
		// 通过 CombatInterface 获取“武器”网格（例如手持法杖/武器Mesh）
		if (USkeletalMeshComponent* Weapon = ICombatInterface::Execute_GetWeapon(OwnerCharacter))
		{
			// 构造忽略数组：至少忽略自己，避免自相碰到
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(OwnerCharacter);

			// 命中结果
			FHitResult HitResult;

			// 以武器的 Socket（TipSocket）位置作为射线起点（光束/子弹发射口）
			const FVector StartLocation = Weapon->GetSocketLocation(FName("TipSocket"));

			// 用小球体（半径10）从起点到目标点扫射一次（可更稳定命中细小目标）
			UKismetSystemLibrary::SphereTraceSingle(
				OwnerCharacter,          // WorldContextObject/Actor（用于定位世界）
				StartLocation,           // 起点（通常为武器口）
				BeamTargetLocation,      // 终点（鼠标/准星推导出的期望目标位置）
				10.f,                    // 半径（单位厘米）
				TraceTypeQuery1,         // Trace 类型（建议替换为具名的自定义通道）
				false,                   // 是否追踪复杂碰撞（一般false）
				ActorsToIgnore,          // 忽略的Actor列表（避免打到自己）
				EDrawDebugTrace::None,   // 调试绘制（可改 ForDuration 看轨迹）
				HitResult,               // 输出命中结果
				true                     // 忽略自身（Self）复杂情况的附加过滤
			);

			// 若有阻挡命中：以“冲击点”（ImpactPoint）作为精确命中位置
			if (HitResult.bBlockingHit)
			{
				// 更新光束目标坐标（命中点通常比原“目标位置”更准确）
				MouseHitLocation = HitResult.ImpactPoint;
				// 记录命中的 Actor（可用于后续伤害/对齐/特效等）
				MouseHitActor = HitResult.GetActor();
			}
		}
	}
}

void UAuraBeamSpell::StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets)
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
	ActorsToIgnore.Add(MouseHitActor);

	TArray<AActor*> OverlappingActors;
	UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(
		GetAvatarActorFromActorInfo(),
		OverlappingActors,
		ActorsToIgnore,
		850.f,
		MouseHitActor->GetActorLocation()
		);

	int32 NumAdditionTargets = 5;
	UAuraAbilitySystemLibrary::GetClosestTargets(NumAdditionTargets, OverlappingActors, OutAdditionalTargets, MouseHitActor->GetActorLocation());
}

