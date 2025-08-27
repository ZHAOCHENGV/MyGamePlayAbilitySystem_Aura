


#include "Character/CharacterBase.h"

#include "AbilitySystemComponent.h"
#include "AuraGamePlayTags.h"
#include "Abilities/GameplayAbility.h"
#include "Aura/Aura.h"
#include "Components/CapsuleComponent.h"
#include "Debuff/DebuffNiagaraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/AuraAbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


ACharacterBase::ACharacterBase()
{
 	PrimaryActorTick.bCanEverTick = false;

	BurnDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>(TEXT("BurnDebuffComponent"));
	BurnDebuffComponent->SetupAttachment(GetRootComponent());
	BurnDebuffComponent->DebuffTag = FAuraGamePlayTags::Get().Debuff_Burn;
	
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(),("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile,ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	//设置移动速度
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	

}

//获取使用的GAS组件
UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ACharacterBase::Die()
{
	// 将角色的武器从其附着的组件上分离
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	// 调用多播函数处理死亡相关逻辑
	MulticastHandleDeath();
}

bool ACharacterBase::IsDead_Implementation() const
{
	return bDead;
}

AActor* ACharacterBase::GetAvatar_Implementation()
{
	return this;
}

UNiagaraSystem* ACharacterBase::GetBloodEffect_Implementation()
{
	return BloodEffect;
}

FTaggedMontage ACharacterBase::GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag)
{
	// 遍历存储的所有攻击蒙太奇配置
	for (FTaggedMontage TaggedMontage : AttackMontages)
	{
		// 检查当前元素的标签是否与目标标签匹配
		if (TaggedMontage.MontageTag == MontageTag)
		{
			// 找到匹配项，立即返回对应的蒙太奇配置
			return TaggedMontage;
		}
	}
	// 未找到匹配项时返回空结构（默认构造）
	return FTaggedMontage();
}

int32 ACharacterBase::GetMinionCount_Implementation()
{
	return MinionCount;
}

void ACharacterBase::IncrementMinionCount_Implementation(int32 Amount)
{
	MinionCount += Amount;
}

ECharacterClass ACharacterBase::GetCharacterClass_Implementation()
{
	return CharacterClass;
}

FOnASCRegistered ACharacterBase::GetOnASCRegisteredDelegate()
{
	return OnAscRegistered;
}

FOnDeathSignature& ACharacterBase::GetOnDeathSignatureDelegate()
{
	return OnDeathDelegate;
}

USkeletalMeshComponent* ACharacterBase::GetWeapon_Implementation()
{
	return Weapon;
}


void ACharacterBase::MulticastHandleDeath_Implementation ()
{
	//播放死亡音效
	UGameplayStatics::PlaySoundAtLocation(this,DeathSound,GetActorLocation());
	// 使武器启用物理模拟和重力
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	// 使角色的骨骼网格体启用物理模拟和重力
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);

	// 设置骨骼网格体仅启用物理碰撞
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	
	// 使骨骼网格体对静态物体（如地面或墙壁）产生碰撞响应
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	// 禁用胶囊体的碰撞，避免角色死亡后还能被检测到
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Dissolve();
	//设置死亡变量为True
	bDead = true;
	//销毁Buff特效
	BurnDebuffComponent->Deactivate();
	/*//调用死亡委托
	OnDeath.Broadcast(this);*/
	//调用死亡委托
	OnDeathDelegate.Broadcast(this);
	
}

TArray<FTaggedMontage> ACharacterBase::GetAttackMontages_Implementation()
{
	return AttackMontages;
}

void ACharacterBase::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bIsStunned = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bIsStunned ? 0.f : BaseWalkSpeed;
}


void ACharacterBase::OnRep_Stunned()
{
	
}

/**
 * @brief 注册本类需要参与网络复制的属性（如眩晕状态 bIsStunned）
 *
 * @param OutLifetimeProps 引擎用于收集复制规则的数组（向其中添加本类的复制条目）
 *
 * 功能说明：
 * - UE 会在初始化时调用该函数，子类在此将需要复制的 UPROPERTY 注册给引擎。
 * - 未在此处注册的属性，即使标了 UPROPERTY(Replicated)，也不会真正复制。
 *
 * 详细流程：
 * 1) 先调用父类版本，确保父类已注册的复制属性不丢失；
 * 2) 使用 DOREPLIFETIME/DOREPLIFETIME_CONDITION 将本类属性追加到 OutLifetimeProps；
 * 3) 引擎据此在运行时完成属性的网络同步。
 *
 * 注意事项：
 * - 函数签名必须与 AActor 声明一致：参数类型与顺序、末尾 **const** 都不能改；
 * - 对应的属性必须是 UPROPERTY(Replicated) 或 ReplicatedUsing=OnRep_XXX；
 * - Actor 自身需启用复制（构造函数中 bReplicates = true）。
 */
void ACharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// 步骤 1：调用父类实现，保留父类的复制设置
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); // 父类中已注册的属性继续生效

	// 步骤 2：注册本类需要复制的属性
	DOREPLIFETIME(ACharacterBase, bIsStunned);          // 将 bIsStunned 加入复制列表（默认条件：始终复制）
	// 如需条件复制，可用：
	// DOREPLIFETIME_CONDITION(ACharacterBase, bIsStunned, COND_SkipOwner);
}
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void ACharacterBase::InitAbilityActorInfo()
{
	
}

FVector ACharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag)
{
	const FAuraGamePlayTags& GamePlayTags = FAuraGamePlayTags::Get();
	if (MontageTag.MatchesTagExact(GamePlayTags.CombatSocket_Weapon) && IsValid(Weapon))
	{
		//返回获取武器插槽名称的位置
		return Weapon->GetSocketLocation(WeaponTipSocketName);
	}
	if (MontageTag.MatchesTagExact(GamePlayTags.CombatSocket_RightHand) )
	{
		//返回获取右手插槽名称的位置
		return Weapon->GetSocketLocation(RightHandSocketName);
	}
	if (MontageTag.MatchesTagExact(GamePlayTags.CombatSocket_LeftHand) )
	{
		//返回获取左手插槽名称的位置
		return Weapon->GetSocketLocation(LeftHandSocketName);
	}
	if (MontageTag.MatchesTagExact(GamePlayTags.CombatSocket_Tail) )
	{
		//返回尾部插槽名称的位置
		return Weapon->GetSocketLocation(TailSocketName);
	}
	return FVector();

}


// 初始化角色的默认属性
void ACharacterBase::InitializeDefaultAttributes() const
{
	//初始化主要属性
	ApplyEffectToSelf(DefaultPrimaryAttributes,1.f);
	//初始化次要属性
	ApplyEffectToSelf(DefaultSecondaryAttributes,1.f);
	//初始化重要属性
	ApplyEffectToSelf(DefaultSignificantAttributes,1.f);
}

void ACharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GamePlayEffectClass, float Level) const
{
	// 检查 Ability System 组件是否有效
	check(IsValid(GetAbilitySystemComponent()));
	// 检查 GameEffectClass 是否被设置
	check(GamePlayEffectClass);
	// 创建一个 Gameplay Effect 的上下文，通常用于指定应用效果的目标信息
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	//添加源对象
	ContextHandle.AddSourceObject(this);
	// 创建一个用于应用的 Gameplay Effect Spec，这里使用 GameEffectClass 作为默认效果，Level为强度，ContextHandle 为上下文
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GamePlayEffectClass,Level,ContextHandle);
	// 应用这个效果到当前的 Ability System 组件（目标为当前角色）
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(),GetAbilitySystemComponent());
}

void ACharacterBase::AddCharacterAbilities()
{
	//获取能力组件
	UAuraAbilitySystemComponent * AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	//PS:添加角色能力组件应该在服务器上
	//如果没有权限，则退出
	if(!HasAuthority()) return;
	//运行能力组件中的添加角色组件函数
	AuraASC->AddCharacterAbilities(StartupAbilities);
	//添加被动技能能力
	AuraASC->AddCharacterPassiveAbilities(StartupPassiveAbilities);
}

UAnimMontage* ACharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

void ACharacterBase::Dissolve()
{
	// 检查角色的溶解材质实例是否有效
	if(IsValid(DissolveMaterialInstance))
	{
		// 动态创建一个基于溶解材质的动态材质实例
		UMaterialInstanceDynamic* DynamicMatInst = UMaterialInstanceDynamic::Create(DissolveMaterialInstance,this);
		// 将动态材质应用到角色模型的第一个材质槽（索引为0）
		GetMesh()->SetMaterial(0,DynamicMatInst);
		// 启动角色溶解效果的时间轴动画
		StartDissolveTimeLine(DynamicMatInst);
	}
	if (IsValid(WeaponDissolveMaterialInstance))
	{
		UMaterialInstanceDynamic* DynamicMatInst = UMaterialInstanceDynamic::Create(DissolveMaterialInstance,this);
		Weapon->SetMaterial(0,DynamicMatInst);
		StartWeaponDissolveTimeLine(DynamicMatInst);
	}
}


