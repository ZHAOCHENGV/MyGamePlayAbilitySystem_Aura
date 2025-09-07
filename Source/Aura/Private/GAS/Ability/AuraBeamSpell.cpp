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
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(MouseHitActor))
	{
		if (!CombatInterface->GetOnDeathSignatureDelegate().IsAlreadyBound(this, &UAuraBeamSpell::PirmaryTargetDied))
		{
			CombatInterface->GetOnDeathSignatureDelegate().AddDynamic(this, &UAuraBeamSpell::PirmaryTargetDied);
		}
	}
}

/**
 * @brief 采集“额外链对象”（附加目标）：在鼠标命中点附近筛选候选单位，取最近的 N 个并注册“死亡回调”
 *
 * @param OutAdditionalTargets 输出：最终挑选出的附加目标数组（按距离从近到远）
 *
 * 功能说明：
 * - 以鼠标命中目标 MouseHitActor 为中心，半径 850cm 搜“活着的玩家/敌人”（排除自身与命中者）；
 * - 依据技能等级与上限 MaxNumShockTargets，选出最多 N = min(AbilityLevel-1, MaxNumShockTargets) 个最近单位；
 * - 为每个附加目标尝试绑定“死亡事件回调”，以便目标死亡时断开电弧/清理效果。
 *
 * 详细流程：
 * 1) 组建忽略列表（自身、命中者）；2) Overlap 搜索存入 OverlappingActors；
 * 3) 计算 N（等级-1 与上限取小）；4) 以命中点为原点，从 OverlappingActors 中筛出“最近 N 个”到 OutAdditionalTargets；
 * 5) 遍历 OutAdditionalTargets，为其绑定“死亡回调”（避免重复绑定）。
 *
 * 注意事项：
 * - 当 AbilityLevel==1 时，N=0，无附加目标；若等级可能小于 1，需额外 Clamp 以防负数。
 * - 本函数假设 MouseHitActor 有效；若可能为空，使用前应判空或早退。
 * - 当前绑定回调时对 MouseHitActor 进行了 Cast（而不是对循环中的 Target），逻辑上可能有误（见文末“建议 1”）。
 */
void UAuraBeamSpell::StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets)
{
	// 步骤 1：忽略列表——把“自己”和“第一目标（鼠标命中者）”加入忽略，避免被筛进附加目标
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());                    // 忽略施法者自身
	ActorsToIgnore.Add(MouseHitActor);                                    // 忽略第一击中的对象

	// 步骤 2：在命中点附近收集“活体单位”候选（已过滤忽略对象）
	TArray<AActor*> OverlappingActors;
	UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(
		GetAvatarActorFromActorInfo(),                                     // World/上下文（或作为查询者）
		OverlappingActors,                                                 // 输出：候选列表
		ActorsToIgnore,                                                    // 输入：忽略对象
		850.f,                                                             // 半径
		MouseHitActor->GetActorLocation()                                  // 圆心：命中对象的位置
	);

	// 步骤 3：计算“需要的附加目标数” = min(能力等级-1, 系统上限)
	int32 NumAdditionTargets = FMath::Min(GetAbilityLevel() - 1, MaxNumShockTargets);
	// int32 NumAdditionTargets = 5;                                       // 旧的固定写法（注释保留用于对照）

	// 步骤 4：从候选里挑出“距离命中点最近”的 N 个单位
	UAuraAbilitySystemLibrary::GetClosestTargets(
		NumAdditionTargets,                                                // 需要的目标数
		OverlappingActors,                                                 // 候选池
		OutAdditionalTargets,                                              // 输出：最终附加目标
		MouseHitActor->GetActorLocation()                                  // 参考原点：命中点
	);

	// 步骤 5：为每个附加目标绑定“死亡回调”，用于链路断开/清理
	for (AActor* Target : OutAdditionalTargets)
	{
		// ⚠ 这里对 MouseHitActor 做了 Cast，而不是对 Target；通常应对 “Target” 进行绑定（见“建议 1”）
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(MouseHitActor))
		{
			// 若未绑定过则绑定，避免重复
			if (!CombatInterface->GetOnDeathSignatureDelegate().IsAlreadyBound(this, &UAuraBeamSpell::AdditionalTargetDied))
			{
				CombatInterface->GetOnDeathSignatureDelegate().AddDynamic(this, &UAuraBeamSpell::AdditionalTargetDied);
			}
		}
	}
}


