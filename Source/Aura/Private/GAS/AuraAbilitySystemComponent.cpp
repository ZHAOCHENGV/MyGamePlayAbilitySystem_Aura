// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AuraAbilitySystemComponent.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	// 将 OnGameplayEffectAppliedDelegateToSelf 事件委托绑定到当前对象（this）和 EffectApplied() 函数。
	// 这意味着当 Gameplay Effect 被应用于该组件时，会调用 EffectApplied() 函数。
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this,&UAuraAbilitySystemComponent::EffectApplied);
}

void UAuraAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	// 这里将处理应用于当前组件的游戏效果时的逻辑。
	// 可以通过 EffectSpec 获取效果的详细信息，
	// 通过 ActiveEffectHandle 管理当前活动效果。
	GEngine->AddOnScreenDebugMessage(1,3.f,FColor::Red,"EffectSpec Applied");
}
