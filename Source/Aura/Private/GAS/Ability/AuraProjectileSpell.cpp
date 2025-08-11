// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/AuraProjectileSpell.h"


#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AuraGamePlayTags.h"
#include "Actor/AuraProjectile.h"
#include "Interation/CombatInterface.h"


void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation,const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
{


		// 检查当前是否在服务器上运行（仅在服务器上允许生成投射物）
		const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
		if(!bIsServer)return;
		// 获取实现了 ICombatInterface 接口的角色对象（通常是玩家或AI）
		ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	
		// 获取投射物生成的起始位置（通常是角色的某个武器或手部的 Socket 位置）
		const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(),SocketTag);
		//DrawDebugSphere(GetWorld(), SocketLocation, 10.0f, 12, FColor::Red, false, 2.0f);
		// 计算投射物生成的旋转方向
		// Rotation 是从起始位置（SocketLocation）指向目标位置（ProjectileTargetLocation）的旋转
		FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
		//是否开启Pitch轴偏移
		if(bOverridePitch)
		{
			Rotation.Pitch = PitchOverride;
		}
		// 初始化用于生成投射物的变换信息（位置和旋转）
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rotation.Quaternion());

		// 延迟生成投射物（使用 SpawnActorDeferred 允许在生成之前进行额外设置）
		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,                                  // 投射物的类
			SpawnTransform,                                  // 生成时的位置和旋转
			GetOwningActorFromActorInfo(),                  // 投射物的拥有者
			Cast<APawn>(GetOwningActorFromActorInfo()),     // 投射物的“实例化者”（通常是 Pawn，表示角色）
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn // 碰撞处理方法（始终生成）
		);

		//设置投射物默认伤害效果参数
		Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		
		// 完成投射物的生成，并应用最终的生成变换信息
		Projectile->FinishSpawning(SpawnTransform);
	
		
	
	
}

