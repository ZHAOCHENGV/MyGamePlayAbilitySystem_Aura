// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerState.h"

#include "GAS/AuraAbilitySystemComponent.h"
#include "GAS/AuraAttributeSet.h"

AAuraPlayerState::AAuraPlayerState()

{
	//网络更新频率
	NetUpdateFrequency = 100.f;

	//初始化AbilitySystemComponent组件
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	//设置AbilitySystemComponent组件为可复制
	AbilitySystemComponent->SetIsReplicated(true);
	//设置AbilitySystemComponent组件的复制模式
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	//创建AttributeSet属性集合
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
	
}

UAbilitySystemComponent* AAuraPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
