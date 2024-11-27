// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AuraAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

UAuraAttributeSet::UAuraAttributeSet()
{
	//ATTRIBUTE_ACCESSORS宏自动创建的初始化函数
	InitHealth(100.f);
	InitMaxHealth(GetHealth());
	InitMana(100.f);
	InitMaxMana(GetMana());
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


