// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Aura/Aura.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GAS/AuraAbilitySystemLibrary.h"
#include "Kismet/GameplayStatics.h"


AAuraProjectile::AAuraProjectile()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 550.F;
	ProjectileMovement->MaxSpeed = 550.F;
	ProjectileMovement->ProjectileGravityScale = 0.F;
	

}


void AAuraProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeSpan);
	LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound,GetRootComponent());
	//在组件开始重叠时，执行OnSphereOverlap
	Sphere->OnComponentBeginOverlap.AddDynamic(this,&AAuraProjectile::OnSphereOverlap);
	
}

void AAuraProjectile::OnHit()
{
	// 在投射物被销毁时播放音效（ImpactSound）和位置特效（ImpactEffect）
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(),FRotator::ZeroRotator);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,ImpactEffect,GetActorLocation());
	if (LoopingSoundComponent)LoopingSoundComponent->Stop();
	bHit = true;
}

void AAuraProjectile::Destroyed()
{
	// 如果投射物未命中目标（bHit 为 false）且当前没有服务器权限（客户端）
	if (!bHit && !HasAuthority())OnHit();

	// 调用父类的销毁逻辑，确保基础行为被执行
	Super::Destroyed();
}

/**
 * @brief 投射物重叠回调：敌我过滤、触发命中表现，并在服务器侧应用伤害
 *
 * @param OverlappedComponent 本体的碰撞组件
 * @param OtherActor          被命中的Actor
 * @param OtherComp           对方的碰撞组件
 * @param OtherBodyIndex      重叠体索引
 * @param bFromSweep          是否为Sweep造成的重叠
 * @param SweepResult         若为Sweep，包含命中信息（命中点、法线等）
 *
 * 功能说明：
 * - 忽略自身命中，按阵营过滤友军；首次命中触发命中表现；仅服务器应用伤害GE并销毁投射物，客户端做本地命中标记。
 *
 * 详细流程：
 * 1) 获取伤害来源Avatar，与OtherActor相同则忽略；2) 敌我过滤，不是敌对则返回；
 * 3) 首次命中时触发OnHit（播放特效/停止移动等）；4) 服务器侧获取目标ASC并应用伤害；5) 服务器销毁投射物；客户端仅设置bHit。
 *
 * 注意事项：
 * - 伤害应用需在服务器侧进行；确保DamageEffectParams.SourceAbilitySystemComponent有效。
 * - bHit用于防重入的命中表现控制；命中点可用SweepResult传给特效系统以定位爆点。
 */
void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 伤害来源Avatar（用于自伤过滤与阵营判定）
	AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();

	// 忽略命中自身（投射物持有者）
	if (SourceAvatarActor == OtherActor) return;

	// 阵营过滤：非敌对则直接返回
	if (!UAuraAbilitySystemLibrary::IsNotFriend(SourceAvatarActor, OtherActor)) return;

	// 首次命中触发表现（如爆炸/停飞），避免未命中时多次播放
	if (!bHit) OnHit();

	// 仅服务器侧负责应用伤害与销毁
	if (HasAuthority())
	{
		// 获取目标ASC，填充参数并应用伤害GE
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			DamageEffectParams.TargetAbilitySystemComponent = TargetASC; // 设置目标ASC
			UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams); // 应用伤害GE
		}

		// 服务器销毁投射物（延迟到安全时机执行）
		Destroy();
	}
	else
	{
		// 客户端仅做命中标记，避免重复本地表现
		bHit = true;
	}
}
