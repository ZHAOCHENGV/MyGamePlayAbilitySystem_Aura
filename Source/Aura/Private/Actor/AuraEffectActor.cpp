// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraEffectActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/AuraAttributeSet.h"


AAuraEffectActor::AAuraEffectActor()
{

	//创建根组件
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("ScentRoot")));

}



void AAuraEffectActor::BeginPlay()
{

}

void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	//获取目标是否有ASC组件
	//UAbilitySystemBlueprintLibrary 是GAS系统的静态函数库内的函数
	UAbilitySystemComponent * TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC==nullptr)return;

	//检查游戏效果
	check(GameplayEffectClass)
	// 创建游戏效果上下文（EffectContext），存储关于效果应用的信息
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	// 添加源对象信息（通常是效果的触发者，用于效果溯源）
	EffectContextHandle.AddSourceObject(this);
	// 创建效果规范（EffectSpec），定义效果的具体表现（如强度、持续时间等）,1代表效果等级
	FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass,1.F,EffectContextHandle);
	// 应用效果规范到目标自身
	// Data是一个 TSharedPtr<FGameplayEffectSpec> 类型的智能指针
	// EffectSpecHandle.Data.Get从智能指针中提取裸指针。
	// *EffectSpecHandle.Data.Get()解引用裸指针获取实际对象。
	TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

	
	
}


