


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

// 初始化角色的默认属性
void ACharacterBase::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes,1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes,1.f);
}

void ACharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GamePlayEffectClass, float Level) const
{
	// 检查 Ability System 组件是否有效
	check(IsValid(GetAbilitySystemComponent()));
	// 检查 GameEffectClass 是否被设置
	check(GameEffectClass);
	// 创建一个 Gameplay Effect 的上下文，通常用于指定应用效果的目标信息
	const FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	// 创建一个用于应用的 Gameplay Effect Spec，这里使用 GameEffectClass 作为默认效果，Level为强度，ContextHandle 为上下文
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameEffectClass,Level,ContextHandle);
	// 应用这个效果到当前的 Ability System 组件（目标为当前角色）
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(),GetAbilitySystemComponent());
}


