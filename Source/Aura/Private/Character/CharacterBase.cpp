


#include "Character/CharacterBase.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"


ACharacterBase::ACharacterBase()
{
 	PrimaryActorTick.bCanEverTick = false;
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(),("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	

}

//获取使用的GAS组件
UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}


void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void ACharacterBase::InitAbilityActorInfo()
{
	
}

// 初始化角色的主要属性，通过应用一个默认的 Gameplay Effect 来设置初始值
void ACharacterBase::InitializePrimaryAttributes() const
{
	// 检查 Ability System 组件是否有效
	check(IsValid(GetAbilitySystemComponent()));
	// 检查 DefaultPrimaryAttributes 是否被设置
	check(DefaultPrimaryAttributes);
	// 创建一个 Gameplay Effect 的上下文，通常用于指定应用效果的目标信息
	const FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	// 创建一个用于应用的 Gameplay Effect Spec，这里使用 DefaultPrimaryAttributes 作为默认效果，1.f 为强度，ContextHandle 为上下文
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(DefaultPrimaryAttributes,1.f,ContextHandle);
	// 应用这个效果到当前的 Ability System 组件（目标为当前角色）
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(),GetAbilitySystemComponent());
}


