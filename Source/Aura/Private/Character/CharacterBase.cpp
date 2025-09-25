


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


void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	EffectAttachComponent->SetWorldRotation(FRotator::ZeroRotator);
}

ACharacterBase::ACharacterBase()
{
 	PrimaryActorTick.bCanEverTick = true;

	BurnDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>(TEXT("BurnDebuffComponent"));
	BurnDebuffComponent->SetupAttachment(GetRootComponent());
	BurnDebuffComponent->DebuffTag = FAuraGamePlayTags::Get().Debuff_Burn;

	StunDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>(TEXT("StunDebuffComponent"));
	StunDebuffComponent->SetupAttachment(GetRootComponent());
	StunDebuffComponent->DebuffTag = FAuraGamePlayTags::Get().Debuff_Stun;

	
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(),("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile,ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	//设置移动速度
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

	EffectAttachComponent = CreateDefaultSubobject<USceneComponent>(TEXT("EffectAttachPoint"));
	EffectAttachComponent->SetupAttachment(GetRootComponent());
	HaloOfProtectionNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>(TEXT("HaloOfProtectionNiagaraComponent"));
	HaloOfProtectionNiagaraComponent->SetupAttachment(EffectAttachComponent);
	LifeSiphonNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>(TEXT("LifeSiphonNiagaraComponent"));
	LifeSiphonNiagaraComponent->SetupAttachment(EffectAttachComponent);
	ManaSiphonNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>(TEXT("ManaSiphonNiagaraComponent"));
	ManaSiphonNiagaraComponent->SetupAttachment(EffectAttachComponent);
	

}

//获取使用的GAS组件
UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

float ACharacterBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,class AController* EventInstigator, AActor* DamageCauser)
{
	
	const float DamageTake = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	OnDamageDelegate.Broadcast(DamageTake);
	return DamageTake;
	
}

void ACharacterBase::Die(const FVector& DeathImpulse)
{
	// 将角色的武器从其附着的组件上分离
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	// 调用多播函数处理死亡相关逻辑
	MulticastHandleDeath(DeathImpulse);
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


/**
 * @brief (接口实现) 根据一个 GameplayTag 在角色的蒙太奇配置数组中查找并返回对应的 FTaggedMontage。
 * @param MontageTag 要查找的蒙太奇的唯一标识标签。
 * @return 如果找到，返回包含蒙太奇资源、标签和其他相关数据的 FTaggedMontage 结构体副本。如果找不到，返回一个默认构造的空结构体。
 *
 * @par 功能说明
 * 这是一个接口函数的原生 C++ 实现 (`_Implementation`)。它的核心作用是从一个预先配置好的 `AttackMontages` 数组中，
 * 通过 Gameplay Tag 检索出一个特定的动画蒙太奇（Animation Montage）及其相关数据。
 *
 * @par 使用场景
 * 在 Gameplay Ability (技能) 的实现中，当一个攻击技能被激活时，它可能需要播放一个特定的攻击动画。
 * 该技能可以通过调用这个接口函数，并传入自身的技能 Tag（例如 `Abilities.Melee.PrimaryAttack`），
 * 来从角色身上获取应该播放的那个动画蒙太奇资源。这种方式将技能逻辑与具体的动画资源解耦，
 * 使得不同的角色可以拥有相同的攻击技能，但播放符合自身风格的动画。
 *
 * @par 详细流程
 * 1.  使用基于范围的 for 循环 (range-based for loop) 遍历 `AttackMontages` 这个 TArray。
 * 2.  在每次循环中，将当前 `FTaggedMontage` 元素的 `MontageTag` 与传入的目标 `MontageTag` 进行比较。
 * 3.  如果两个 Tag 完全相等，则说明找到了匹配的配置，立即将该 `FTaggedMontage` 元素的副本返回。
 * 4.  如果循环完成后都没有找到任何匹配的 Tag，则返回一个通过默认构造函数创建的空 `FTaggedMontage` 结构体。
 */
FTaggedMontage ACharacterBase::GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag)
{
	// 使用基于范围的 for 循环遍历 AttackMontages 数组。
	// AttackMontages 是一个 UPROPERTY 的 TArray<FTaggedMontage>，在角色蓝图中进行配置。
	// (注意): `for (FTaggedMontage TaggedMontage ...)` 这里是按值拷贝，对于小结构体问题不大，但按引用更高效。
	for (FTaggedMontage TaggedMontage : AttackMontages)
	{
		// 比较当前遍历到的结构体中的 MontageTag 是否与传入的目标 MontageTag 相等。
		// FGameplayTag 重载了 == 操作符，可以进行高效的比较。
		if (TaggedMontage.MontageTag == MontageTag)
		{
			// 如果找到匹配项，立即返回这个结构体的副本，函数执行结束。
			return TaggedMontage;
		}
	}
	// 如果循环正常结束（即没有在中间 return），说明没有在数组中找到任何匹配的 Tag。
	// 此时返回一个默认构造的 FTaggedMontage。其内部的指针将是 nullptr，Tag 将是空的。
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

FOnASCRegistered& ACharacterBase::GetOnASCRegisteredDelegate()
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

void ACharacterBase::SetIsBeingShocked_Implementation(bool bInShock)
{
	bIsBeingShocked = bInShock;
}

bool ACharacterBase::IsBeingShocked_Implementation() const
{
	return bIsBeingShocked;
}

FOnDamageSignature& ACharacterBase::GetOnDamageSignature()
{
	return OnDamageDelegate;
}

/**
 * @brief (NetMulticast RPC) 在服务器和所有客户端上执行角色的死亡表现逻辑。
 * @param DeathImpulse 施加到角色尸体和武器上的物理冲击力，用于制作被击飞的布娃娃效果。
 *
 * @par 功能说明
 * 这是一个多播（NetMulticast）RPC (Remote Procedure Call) 的实现。当服务器上的角色死亡时，
 * 服务器会调用这个函数。引擎的网络系统会自动将这个调用请求转发给所有已连接的客户端，
 * 最终使得这个函数在服务器和所有客户端上都被执行一遍。
 *
 * 它的核心目的是确保所有玩家都能看到一致的死亡表现：
 * - 听到死亡音效。
 * - 看到武器和尸体符合物理规律地倒下或飞出。
 * - 看到溶解 (Dissolve) 等死亡特效。
 * - 本地游戏逻辑（如 UI 更新）能够响应死亡事件。
 *
 * @par 详细流程
 * 1.  **播放音效**: 在角色死亡的位置播放一个 3D 死亡音效。
 * 2.  **处理武器**:
 *     - 开启武器模型的物理模拟和重力，让它从角色手中掉落。
 *     - 将其碰撞模式设为 `PhysicsOnly`，意味着它只参与物理计算，不再触发重叠等游戏性事件。
 *     - 对武器施加一个较小的冲击力，让它有一个自然的飞出效果。
 * 3.  **处理角色身体 (布娃娃)**:
 *     - 开启角色骨骼网格体 (`Mesh`) 的物理模拟和重力，即“开启布娃娃 (Ragdoll)”。
 *     - 将其碰撞模式设为 `PhysicsOnly`。
 *     - 关键步骤：设置其对 `ECC_WorldStatic`（如地面、墙壁）的碰撞响应为 `Block`，确保尸体能正确地与场景发生碰撞，而不是穿透地面。
 *     - 对尸体施加主要的 `DeathImpulse`，使其被击飞。
 * 4.  **清理碰撞**: 禁用角色胶囊体 (`CapsuleComponent`) 的碰撞。这是非常重要的一步，可以防止活着的角色被尸体“绊倒”，或者技能依然能命中没有实际意义的尸体。
 * 5.  **播放特效**: 调用 `Dissolve()` 函数，开始播放一个溶解或消失的视觉特效。
 * 6.  **设置状态**: 将 `bDead` 标志设为 `true`。
 * 7.  **清理 Debuff 特效**: 停用（`Deactivate`）附着在角色身上的燃烧和眩晕等粒子特效组件。
 * 8.  **广播本地委托**: 调用 `OnDeathDelegate.Broadcast(this)`。这是一个本地的委托，用于通知该角色实例上的其他 C++ 或蓝图逻辑（例如，更新此角色头顶的血条 UI）“这个角色已经死了”。
 */
// `_Implementation` 后缀表明这是 UFUNCTION(NetMulticast) RPC 的 C++ 实现。
// 这个函数会在服务器上被调用，然后自动在所有客户端上执行。
void ACharacterBase::MulticastHandleDeath_Implementation (const FVector& DeathImpulse)
{
	// --- 音效和物理表现 (所有客户端都能看到/听到) ---
	
	// 在角色当前位置播放死亡音效。
	UGameplayStatics::PlaySoundAtLocation(this,DeathSound,GetActorLocation());
	// 让武器掉落并与世界进行物理交互。
	Weapon->SetSimulatePhysics(true);// 开启物理模拟。
	Weapon->SetEnableGravity(true); // 开启重力。
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);// 设置为纯物理碰撞体。
	Weapon->AddImpulse(DeathImpulse * 0.1f, NAME_None, true);// 给武器一个较小的冲击力。`true` 表示速度变更。

	// 开启角色的布娃娃效果。
	GetMesh()->SetSimulatePhysics(true);// 开启骨骼网格体的物理模拟。
	GetMesh()->SetEnableGravity(true);// 开启重力。
	// 设置骨骼网格体仅启用物理碰撞
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);// 设置为纯物理碰撞体。
	// (为什么这么做): 默认情况下，物理模拟的物体可能不会与所有东西都碰撞。
	// 这一行明确地告诉引擎：“这个尸体必须被墙和地面挡住。”
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetMesh()->AddImpulse(DeathImpulse, NAME_None, true);// 给身体施加主要的死亡冲击力。


	// --- 清理和状态更新 ---

	// 禁用胶囊体的碰撞。这是为了防止尸体依然能阻挡其他活着的角色。
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// 调用一个自定义函数来开始播放溶解特效。
	Dissolve();
	
	// 设置死亡状态标志。
	bDead = true;
	
	// 停用并隐藏燃烧和眩晕的粒子特效。
	BurnDebuffComponent->Deactivate();
	StunDebuffComponent->Deactivate();
	
	// (为什么这么做): OnDeathDelegate 是一个本地的委托。
	// 它会在服务器和每一个客户端上各自广播。
	// 任何绑定到这个委托的本地系统（比如这个角色的UI控制器）都会收到通知。
	// 这与网络复制无关，纯粹是本地事件通知。
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

void ACharacterBase::OnRep_Burned()
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
	DOREPLIFETIME(ACharacterBase, bIsBurned);           // 将 bIsBurned 加入复制列表（默认条件：始终复制）
	DOREPLIFETIME(ACharacterBase, bIsBeingShocked);		// 将 bIsBeingShocked 加入复制列表（默认条件：始终复制）
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


/**
 * @brief (接口实现) 根据一个 GameplayTag 返回角色身上一个指定的战斗相关插槽 (Socket) 的世界坐标。
 * @param MontageTag 一个 GameplayTag，用于标识请求的是哪个位置 (例如：武器尖、左手、右手等)。
 * @return 返回所请求插槽的世界坐标 FVector。如果 Tag 无效或对应组件不存在，则返回零向量 FVector()。
 *
 * @par 功能说明
 * 这是一个接口函数的原生 C++ 实现 (`_Implementation`)。它的核心作用是作为一个统一的查询入口，
 * 允许外部系统（通常是 Gameplay Ability 或特效管理器）通过一个语义化的标签（Tag），
 * 来获取角色骨骼上特定插槽的精确世界位置。
 *
 * @par 使用场景
 * - **技能释放**: 一个火球术技能在激活时，需要知道火球应该从哪里生成。它可以调用这个函数并传入 `CombatSocket_RightHand` Tag，来获取右手手心的位置。
 * - **武器轨迹**: 一个挥砍技能需要开启武器的轨迹特效。它可以调用此函数并传入 `CombatSocket_Weapon` Tag，来获取武器尖的位置作为轨迹的起点。
 * - **击中特效**: 当攻击命中敌人时，需要在敌人身上生成一个受击特效。可以通过射线检测得到击中位置，但如果想在攻击者的武器尖上也生成一个火花特效，就可以调用此函数来获取该位置。
 *
 * @par 详细流程
 * 1.  获取 Gameplay Tag 单例以方便访问。
 * 2.  使用一连串的 `if` 语句，将传入的 `MontageTag` 与预定义的战斗插槽 Tag 进行精确比较。
 * 3.  **武器尖**: 如果 Tag 匹配 `CombatSocket_Weapon`，并且 `Weapon` 组件有效，则返回武器骨骼上名为 `WeaponTipSocketName` 的插槽位置。
 * 4.  **右手/左手/尾巴**: 如果 Tag 匹配其他身体部位，则返回角色主骨骼网格 (`GetMesh()`) 上对应名称 (`RightHandSocketName` 等) 的插槽位置。
 * 5.  **默认返回**: 如果没有任何 Tag 匹配，则返回一个零向量 `FVector()`，表示查询失败。
 *
 * @par 注意事项
 * - `WeaponTipSocketName`, `RightHandSocketName` 等是 `FName` 类型的成员变量，在 C++ 或蓝图中预先设置好。
 * - 这种基于 Tag 的查询方式极大地增强了代码的灵活性和可读性，避免了硬编码字符串或使用枚举。
 */
FVector ACharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag)
{
	// 获取 Gameplay Tag 的单例，方便后续访问预定义的 Tag。
	const FAuraGamePlayTags& GamePlayTags = FAuraGamePlayTags::Get();

	// 检查传入的 Tag 是否精确匹配“武器”插槽的 Tag，并且武器组件的指针是否有效。
	if (MontageTag.MatchesTagExact(GamePlayTags.CombatSocket_Weapon) && IsValid(Weapon))
	{
		// 如果匹配，返回 Weapon 组件上名为 WeaponTipSocketName 的插槽的世界坐标。
		// WeaponTipSocketName 是一个 FName 成员变量，在蓝图中配置。
		return Weapon->GetSocketLocation(WeaponTipSocketName);
	}
	// 检查传入的 Tag 是否精确匹配“右手”插槽的 Tag。
	if (MontageTag.MatchesTagExact(GamePlayTags.CombatSocket_RightHand) )
	{
		// 如果匹配，返回角色主骨骼网格 (Mesh) 上名为 RightHandSocketName 的插槽的世界坐标。
		return GetMesh()->GetSocketLocation(RightHandSocketName); // BUG: 原代码这里也写的 Weapon->... 应该是 GetMesh()->...
	}
	// 检查传入的 Tag 是否精确匹配“左手”插槽的 Tag。
	if (MontageTag.MatchesTagExact(GamePlayTags.CombatSocket_LeftHand) )
	{
		// 如果匹配，返回角色主骨骼网格 (Mesh) 上名为 LeftHandSocketName 的插槽的世界坐标。
		return GetMesh()->GetSocketLocation(LeftHandSocketName); // BUG: 原代码这里也写的 Weapon->... 应该是 GetMesh()->...
	}
	// 检查传入的 Tag 是否精确匹配“尾巴”插槽的 Tag。
	if (MontageTag.MatchesTagExact(GamePlayTags.CombatSocket_Tail) )
	{
		// 如果匹配，返回角色主骨骼网格 (Mesh) 上名为 TailSocketName 的插槽的世界坐标。
		return GetMesh()->GetSocketLocation(TailSocketName); // BUG: 原代码这里也写的 Weapon->... 应该是 GetMesh()->...
	}

	// 如果没有任何 Tag 匹配，返回一个零向量 FVector(0,0,0)。
	// 调用方应该检查返回值是否为零向量来判断查询是否成功。
	
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


