// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/AuraFireBlast.h"

#include "Actor/AuraFireBall.h"
#include "GAS/AuraAbilitySystemLibrary.h"

FString UAuraFireBlast::GetDescription(int32 Level)
{
	
		const int32 ScaleDamage = Damage.GetValueAtLevel(Level);
		const float ManaCost = FMath::Abs(GetManaCost(Level));
		const float Cooldown = GetCooldown(Level);
		
		return FString::Printf(TEXT(
			"<Title>火焰爆发</>\n\n"
			"<Small>等级: %d</>\n"
			"<Small>消耗: </><ManaCost>%.1f</><Small> 法力值</>\n"
			"<Small>冷却: </><Cooldown>%.1f</><Small> 秒</>\n\n"
			"<Default>向四面八方发射%d个火球，在返回时发生爆炸，并照成:</>"
			"<Damage>%d</>"
			"<Default>点带伤害衰减的范围火焰伤害，有几率照成燃烧效果</>"),
			Level,
			ManaCost,
			Cooldown,
			NumFireBalls,
			ScaleDamage);
	
}

FString UAuraFireBlast::GetNextLevelDescription(int32 Level)
{
	const int32 ScaleDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
				"<Title>下一等级火焰爆发</>\n\n"
				"<Small>等级: %d</>\n"
				"<Small>消耗: </><ManaCost>%.1f</><Small> 法力值</>\n"
				"<Small>冷却: </><Cooldown>%.1f</><Small> 秒</>\n\n"
				"<Default>向四面八方发射%d个火球，在返回时发生爆炸，并照成:</>"
				"<Damage>%d</>"
				"<Default>点带伤害衰减的范围火焰伤害，有几率照成燃烧效果</>"),
				Level,
				ManaCost,
				Cooldown,
				NumFireBalls,
				ScaleDamage);
}


/**
 * @brief 以角色为圆心，等角度环形生成若干 FireBall 投射物（延迟生成，先配置再完成生成）
 *
 * @return TArray<AAuraFireBall*> 返回本次生成的所有 FireBall 指针（已 FinishSpawning 完成生成）
 *
 * 功能说明：
 * - 取角色当前位置与前向向量，按 360° 等间距计算若干旋转（NumFireBalls 个），围绕 Up 轴环形布置；
 * - 使用 SpawnActorDeferred 延迟生成，每个投射物先注入伤害参数与归返对象，再调用 FinishSpawning 完成生成；
 * - 返回 FireBall 列表，便于上层做后续管理（例如统一设置速度、注册事件、存入容器等）。
 *
 * 详细流程：
 * 1) 创建返回数组；2) 读取角色 Forward 与 Location；3) 计算等角 Rotators；
 * 4) 遍历 Rotators：4.1 组装 SpawnTransform；4.2 延迟生成 FireBall；4.3 写入 DamageEffectParams 等；
 * 4.4 加入结果数组；4.5 FinishSpawning 完成生成；5) 返回数组。
 *
 * 注意事项：
 * - 仅在服务器 Spawn（HasAuthority）才能复制到客户端；若本函数可能在客户端被调用，需上层先行 Authority 检查。
 * - SpawnActorDeferred 允许在 BeginPlay/构造后、注册前写入属性；但务必调用 FinishSpawning，否则对象不完整。
 * - EvenlySpacedRotators 的实现若在第二次 RotateAngleAxis 时硬编码了 FVector::UpVector，则外部传入 Axis 将被忽略（请核对实现，避免“轴不一致”）。
 * - 建议在 Spawn 前校验 FireBallClass 非空；必要时 Reserve(FireBalls) 以减少重分配。
 */
TArray<AAuraFireBall*> UAuraFireBlast::SpawnFireBalls()
{
	// 创建返回数组（用于收集生成的 FireBall）
	TArray<AAuraFireBall*> FireBalls;

	// 读取角色前向向量（决定初始朝向）                                       
	const FVector ForWard = GetAvatarActorFromActorInfo()->GetActorForwardVector();

	// 读取角色当前位置（作为环形生成的圆心）                                 
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();

	// 计算等角度旋转：围绕 Up 轴把 360° 平均分成 NumFireBalls 份               
	TArray<FRotator> Rotators = UAuraAbilitySystemLibrary::EvenlySpacedRotators(ForWard, FVector::UpVector, 360.f, NumFireBalls);

	// 遍历每个旋转，逐个延迟生成 FireBall                                      
	for (const FRotator& Rotator : Rotators)
	{
		// 组装生成位姿（位置=圆心，旋转=等角朝向）                           
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(Location);
		SpawnTransform.SetRotation(Rotator.Quaternion());

		// 延迟生成投射物：可在 FinishSpawning 前设置默认属性                   
		AAuraFireBall* FireBall = GetWorld()->SpawnActorDeferred<AAuraFireBall>
		(
			FireBallClass,                                    // 要生成的类（应确保在外部已设置为有效 Blueprint/C++ 类）
			SpawnTransform,                                   // 初始变换
			GetOwningActorFromActorInfo(),                   // Owner：通常为拥有该 GA 的 Actor
			CurrentActorInfo->PlayerController->GetPawn(),   // Instigator：用于伤害归属/仇恨（此处取玩家 Pawn）
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn  // 碰撞策略：总是生成
		);

		// 注入该 FireBall 的伤害参数（从 GA 默认配置构造的 Params）              
		FireBall->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

		// 设置“归返对象”（若 FireBall 需要回到施法者或跟随其位置）                
		FireBall->ReturnToActor = GetAvatarActorFromActorInfo();

		FireBall->ExplosionDamageParams = MakeDamageEffectParamsFromClassDefaults();
		FireBall->SetOwner(GetAvatarActorFromActorInfo());
		// 收集到结果数组（此时对象已 spawn 但尚未 FinishSpawning）               
		FireBalls.Add(FireBall);

		// 完成生成（触发构造后逻辑/注册组件/BeginPlay 等）                       
		FireBall->FinishSpawning(SpawnTransform);
	}

	// 返回本次生成的全部 FireBall                                          
	return FireBalls;
}
