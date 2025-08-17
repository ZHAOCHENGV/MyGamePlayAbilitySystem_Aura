// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/AuraFireBolt.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGamePlayTags.h"
#include "Actor/AuraProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GAS/AuraAbilitySystemLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

/**
 * 获取指定等级下火焰箭技能的描述文本
 *
 * @param Level   技能等级
 * @return        格式化后的技能描述字符串
 *
 * 功能说明：
 * 根据传入的技能等级，动态生成火焰箭技能在该等级下的详细描述文本。
 * 描述中包含技能名称、发射火球数量、造成的火焰伤害等信息，支持富文本标签，便于UI高亮显示关键数据。
 *
 * 详细说明：
 * - 首先通过伤害类型映射表，查找火焰伤害在当前等级下的数值（见下方Damage变量详细解释）。
 * - 若为1级，技能名称为“1级火焰箭”，描述中写明“发射一道火焰”，
 *   并展示该等级火焰伤害数值和燃烧概率说明。
 * - 若为2级及以上，技能名称为“X级火焰箭”，描述中写明“发射N道火焰”；
 *   其中N为当前等级和最大弹道数（NumProjectiles）中的较小值，用于限制高等级下的投射物数量不会超过技能设定上限。
 * - 返回拼接好的富文本字符串，便于在UI中高亮显示技能核心效果。
 */
FString UAuraFireBolt::GetDescription(int32 Level)
{
	/*
		* - DamageTypes：这是一个 TMap<FGameplayTag, FAuraDamageInfo> 类型的成员变量，存储了不同伤害类型（如火焰、冰霜等）与其对应的数值成长信息。
		* - FAuraGamePlayTags::Get().Damage_Fire：静态单例，返回全局定义的火焰伤害标签，用于查找火焰类型的伤害信息。
		* - DamageTypes[...]: 通过火焰伤害标签在映射表中查找火焰类型的成长数据FAuraDamageInfo。
		* - GetValueAtLevel(Level)：这是FAuraDamageInfo的成员函数，根据传入的技能等级Level，返回该等级下的具体伤害数值（通常查表或插值实现）。
		* - 所以整句代码的作用是：根据当前技能等级，从技能的伤害成长表中查出对应的火焰伤害数值，用于技能描述或实际计算。
	 */
	const int32 ScaleDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if(Level == 1)
	{
		return FString::Printf(TEXT(
			"<Title>火焰箭</>\n\n"
			"<Small>等级: %d</>\n"
			"<Small>消耗: </><ManaCost>%.1f</><Small> 法力值</>\n"
			"<Small>冷却: </><Cooldown>%.1f</><Small> 秒</>\n\n"
			"<Default>发射一道火焰，撞击时会爆炸并照成:</>"
			"<Damage>%d</>"
			"<Default>点火焰伤害，有几率照成燃烧效果</>"),
			Level,
			ManaCost,
			Cooldown,
			ScaleDamage);
	}
		return FString::Printf(TEXT(
			"<Title>火焰箭</>\n\n"
			"<Small>等级: %d</>\n"
			"<Small>消耗: </><ManaCost>%.1f</><Small> 法力值</>\n"
			"<Small>冷却: </><Cooldown>%.1f</><Small> 秒</>\n\n"
			"<Default>发射%d道火焰，撞击时会爆炸并照成:</>"
			"<Damage>%d</>"
			"<Default>点火焰伤害，有几率照成燃烧效果</>"),
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level,NumProjectiles),
			ScaleDamage);
}

/**
 * 获取下一等级火焰箭技能的描述文本
 *
 * @param Level   技能下一等级
 * @return        下一等级的技能描述字符串
 *
 * 功能说明：
 * 动态生成下一等级火焰箭技能的描述文本，让玩家预览升级后的技能效果。
 * 与当前等级描述类似，但参数为下一等级。
 *
 * 详细说明：
 * - 计算下一等级下的火焰伤害数值。
 * - 描述中写明升级后可发射的火焰弹数量（受最大弹道上限NumProjectiles限制）。
 * - 主要用于技能升级界面，帮助玩家决策是否消耗点数进行升级。
 */
FString UAuraFireBolt::GetNextLevelDescription(int32 Level)
{
	const int32 ScaleDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			"<Title>下一等级火焰箭</>\n\n"
			"<Small>等级: %d</>\n"
			"<Small>消耗: </><ManaCost>%.1f</><Small> 法力值</>\n"
			"<Small>冷却: </><Cooldown>%.1f</><Small> 秒</>\n\n"
			"<Default>发射%d道火焰，撞击时会爆炸并照成:</>"
			"<Damage>%d</>"
			"<Default>点火焰伤害，有几率照成燃烧效果</>"),
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level,NumProjectiles),
			ScaleDamage);
}

/**
 * @brief 在服务器上生成一批可跟踪（Homing）的火焰投射物，朝给定目标位置扇形发射
 * @param ProjectileTargetLocation 目标世界坐标（用于计算初始朝向/无锁定目标时的临时跟踪点）
 * @param SocketTag                发射用 Socket 的 GameplayTag（从角色或武器上取 Socket 位置）
 * @param bOverridePitch           是否覆盖俯仰角 Pitch（用于抬高/压低初始弹道）
 * @param PitchOverride            覆盖时使用的 Pitch 角（度）
 * @param HomingTarget             可选：锁定的跟踪目标 Actor（若为空则使用一个临时 SceneComponent 作为跟踪点）
 * @details
 *  - 【作用】按“扇形均匀分布”的多方向，延迟生成（Deferred Spawn）一组弹体；为每个弹体配置伤害参数与跟踪目标。
 *  - 【背景】Deferred Spawn 允许在 FinishSpawning 前设置构造参数；Homing 通过 ProjectileMovement 的 HomingTargetComponent 指向一个组件。
 *  - 【流程】
 *    1) 仅在服务器生成（HasAuthority）；取 Combat Socket 世界位置；
 *    2) 算出从 Socket 指向目标的旋转，必要时覆盖 Pitch；得到 Forward 向量；
 *    3) 取“有效发射数量”（本地 NumProjectiles 与 Ability 等级取最小值），并根据扇形角生成等间距旋转；
 *    4) 循环：Deferred Spawn 投射物 → 写入伤害参数 → 设置 Homing 目标（锁定 Actor 或临时组件）→ 随机加速度 → 完成生成。
 *  - 【注意】
 *    - 需确保服务器-客户端一致性：只在服务器 Spawn，复制由投射物自身的 Replication 设置完成。
 *    - 若使用临时 SceneComponent 作为 Homing 目标，必须有合适的 Outer/注册（见下方“优化建议”）。
 */
void UAuraFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag,bool bOverridePitch, float PitchOverride, AActor* HomingTarget)
{
	// 检查当前是否在服务器上运行（仅在服务器上允许生成投射物）
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if(!bIsServer)return; // 非服务器则直接返回，避免多端重复生成

	// 获取实现了 ICombatInterface 接口的角色对象（通常是玩家或AI）
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo()); // 可用于扩展（此处未直接使用）
	
	// 获取投射物生成的起始位置（通常是角色的某个武器或手部的 Socket 位置）
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(),SocketTag); // 发射口坐标
	//DrawDebugSphere(GetWorld(), SocketLocation, 10.0f, 12, FColor::Red, false, 2.0f);

	// 计算投射物生成的旋转方向
	// Rotation 是从起始位置（SocketLocation）指向目标位置（ProjectileTargetLocation）的旋转
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation(); // 朝向目标的初始旋转

	//是否开启Pitch轴偏移
	if(bOverridePitch)Rotation.Pitch = PitchOverride; // 覆盖俯仰角（抬高/压低弹道）

	// 将旋转转成方向向量（Forward）
	const FVector Forward = Rotation.Vector(); // 基准前向

	// 计算实际发射数量：不超过技能等级（例如一级只发 1 发）
	const int32 EffectiveNumProjectiles = FMath::Min(NumProjectiles,GetAbilityLevel()); // 发射数量上限

	// 根据扇形角度与数量，生成等间隔的旋转序列（围绕世界 Up 轴展开）
	TArray<FRotator> Rotations = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, ProjectileSpread, EffectiveNumProjectiles); // 扇形旋转集

	// 遍历每个方向，逐个生成投射物
	for (const FRotator Rot : Rotations)
	{
		// 初始化用于生成投射物的变换信息（位置和旋转）
		FTransform SpawnTransform; // 生成变换
		SpawnTransform.SetLocation(SocketLocation); // 设置位置
		SpawnTransform.SetRotation(Rot.Quaternion()); // 设置旋转

		// 延迟生成投射物（使用 SpawnActorDeferred 允许在生成之前进行额外设置）
		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,                                  // 投射物的类
			SpawnTransform,                                  // 生成时的位置和旋转
			GetOwningActorFromActorInfo(),                  // 投射物的拥有者
			Cast<APawn>(GetOwningActorFromActorInfo()),     // 投射物的“实例化者”（通常是 Pawn，表示角色）
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn // 碰撞处理方法（始终生成）
		);
		//设置投射物默认伤害效果参数
		Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults(); // 赋默认伤害参数（GE/等级/SetByCaller等）
		
		// 配置 Homing 目标：优先使用传入的锁定 Actor；否则创建临时跟踪点
		if (HomingTarget && HomingTarget->Implements<UCombatInterface>())
		{
			Projectile->ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent(); // 以目标根组件为 Homing 目标
		}
		else
		{
			Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(USceneComponent::StaticClass()); // 创建临时目标组件（见优化建议）
			Projectile->HomingTargetSceneComponent->SetWorldLocation (ProjectileTargetLocation); // 放在指定世界坐标
			Projectile->ProjectileMovement->HomingTargetComponent = Projectile->HomingTargetSceneComponent; // 使用该组件作为 Homing 目标
		}

		// 设置 Homing 加速度范围（给每发弹体一定随机性）
		Projectile->ProjectileMovement->HomingAccelerationMagnitude = FMath::FRandRange(HomingAccelerationMin, HomingAccelerationMax); // 随机加速度

		// 控制是否启用 Homing（由配置开关决定）
		Projectile->ProjectileMovement->bIsHomingProjectile = bLaunchHomingProjectiles; // 启用/禁用跟踪
		
		// 完成投射物的生成，并应用最终的生成变换信息
		Projectile->FinishSpawning(SpawnTransform); // 结束延迟生成（Spawn 完毕，进入世界）
	}
}

