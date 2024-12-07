// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AuraAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

UAuraAttributeSet::UAuraAttributeSet()
{
	//ATTRIBUTE_ACCESSORS宏自动创建的初始化函数
	InitHealth(50.f);
	InitMaxHealth(100.f);
	InitMana(20.f);
	InitMaxMana(100.f);
	
}

void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//DOREPLIFETIME_CONDITION_NOTIFY 是一个宏，用于添加对指定属性的复制行为。
	//UAuraAttributeSet 是类名。
	//Health 是要复制的属性。
	//COND_None 表示没有特殊的复制条件。
	//REPNOTIFY_Always 表示在属性变化时总是会通知。
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,Health,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,MaxHealth,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,Mana,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,MaxMana,COND_None,REPNOTIFY_Always);
}

void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	//如果Attribute值 == 血量
	if (Attribute == GetHealthAttribute())
	{
		//限制这个值得最大最小值
		NewValue = FMath::Clamp(NewValue,0,GetMaxHealth());
		UE_LOG(LogTemp,Warning,TEXT("Health : %f"),NewValue);
	}
	
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue,0,GetMaxMana());
		UE_LOG(LogTemp,Warning,TEXT("Mana : %f"),NewValue);
	}
	
}

void UAuraAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	//创建数据结构
	FEffectProperties Props;
	//设置效果属性
	SetEffectProperties(Data,Props);
	
}

void UAuraAttributeSet::SetEffectProperties(const struct FGameplayEffectModCallbackData& Data, FEffectProperties& Props)const
{
	// 获取Gameplay Effect的上下文句柄
	Props.EffectContextHandle = Data.EffectSpec.GetContext();
	// 获取原始施加效果的Ability System Component
	UAbilitySystemComponent * SourceASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();
	// 检查SourceASC和相关的Actor是否有效
	if (IsValid(Props.SourceASC) && Props.SourceASC->AbilityActorInfo.IsValid() && Props.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		// 获取施法者的AvatarActor（角色或物体）引用
		Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get();
		// 获取施法者的Controller
		Props.SourceController = Props.SourceASC->AbilityActorInfo->PlayerController.Get();
		// 如果控制器为空，并且AvatarActor有效，则从AvatarActor获取控制器
		if (Props.SourceController == nullptr && Props.SourceAvatarActor!=nullptr)
		{
			if(const APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				// 获取Pawn的控制器
				Props.SourceController = Pawn->GetController();
			}
		}
		// 如果控制器有效，从控制器获取角色信息
		if(Props.SourceController)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}
	}
	// 检查目标的AbilityActorInfo是否有效
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		// 获取目标Actor（受到效果的角色或物体）
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		// 获取目标的Controller
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		// 将目标Actor转换为角色
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		// 获取目标Actor的Ability System Component
		Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}
}

void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	//处理属性在网络上变化后的通知逻辑。
	//GAMEPLAYATTRIBUTE_REPNOTIFY 宏用于处理属性更新，确保任何需要更新的引用（如UI）能正确响应属性的变化。
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,Health,OldHealth);
}

void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	//处理属性在网络上变化后的通知逻辑。
	//GAMEPLAYATTRIBUTE_REPNOTIFY 宏用于处理属性更新，确保任何需要更新的引用（如UI）能正确响应属性的变化。
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,MaxHealth,OldMaxHealth);
}

void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	//处理属性在网络上变化后的通知逻辑。
	//GAMEPLAYATTRIBUTE_REPNOTIFY 宏用于处理属性更新，确保任何需要更新的引用（如UI）能正确响应属性的变化。
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,Mana,OldMana);
}

void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	//处理属性在网络上变化后的通知逻辑。
    //GAMEPLAYATTRIBUTE_REPNOTIFY 宏用于处理属性更新，确保任何需要更新的引用（如UI）能正确响应属性的变化。
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,MaxMana,OldMaxMana);
}


