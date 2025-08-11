// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"


AAuraEffectActor::AAuraEffectActor()
{ 
 PrimaryActorTick.bCanEverTick = false;


}


void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	//如果目标有“敌人”标签，并且对敌人施加效果为False,则退出
	if(TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies ) return; 
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
	FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass,ActorLevel,EffectContextHandle);
	// 应用效果规范到目标自身
	// Data是一个 TSharedPtr<FGameplayEffectSpec> 类型的智能指针
	// EffectSpecHandle.Data.Get从智能指针中提取裸指针。
	// *EffectSpecHandle.Data.Get()解引用裸指针获取实际对象。
	//保存应用效果到一个变量中
	FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

	//获取效果的持续时间类型是否为永久,如果等于永久返回True
	//EffectSpecHandle.Data.Get() 从句柄中获取 FGameplayEffectSpec 的指针。
	//->Def.Get()通过 FGameplayEffectSpec 获取定义信息 UGameplayEffect。
	//->DurationPolicy 从特效定义中访问其持续时间策略。
	const bool bIsInfinite	= EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
	//如果持续效果为永久和无限持续时间的游戏效果的移除政策为 重叠结束时移除
	if (bIsInfinite && InfiniteEEffectRemovalPolicy==EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		//把效果和ASC组件，存储到映射中
		ActiveEffectHandles.Add(ActiveEffectHandle,TargetASC);
	}
	//如果应用时效果为真，且 效果为实时的，则销毁
	if (!bIsInfinite && bDestroyOnEffectApplication)
	{
		Destroy();
	}
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	//如果目标有“敌人”标签，并且对敌人施加效果为False,则退出
	if(TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies ) return; 
	//如果即时效果应用程序策略 等于 重叠，则应用游戏效果
	if (InstantEffectApplicationPolicy==EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor,InstantGameplayEffectClass);
	}
	//如果持续效果应用程序策略 等于 重叠，则应用游戏效果
	if (DurationEffectApplicationPolicy ==EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor,DurationGameplayEffectClass);
	}
	//如果无限效果应用程序策略 等于 重叠，则应用游戏效果
	if (InfiniteEffectApplicationPolicy ==EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor,InfiniteGameplayEffectClass);
	}
	
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	//如果目标有“敌人”标签，并且对敌人施加效果为False,则退出
	if(TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies ) return; 
	//如果即时效果应用程序策略 等于 结束重叠，则应用游戏效果
	if (InstantEffectApplicationPolicy==EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor,InstantGameplayEffectClass);
	}
	//如果持续效果应用程序策略 等于 结束重叠，则应用游戏效果
	if (DurationEffectApplicationPolicy ==EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor,DurationGameplayEffectClass);
	}
	//如果无限效果应用程序策略 等于 重叠，则应用游戏效果
	if (InfiniteEffectApplicationPolicy ==EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor,InfiniteGameplayEffectClass);
	}
	// 如果无限效果的移除策略是“重叠结束时移除”
	if (InfiniteEEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		//获取目标Actor的能力系统组件
		UAbilitySystemComponent * TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		// 如果目标能力系统组件无效，则退出函数
		if (!IsValid(TargetASC))return;
		// 创建一个数组，用于存储需要移除的效果句柄
		TArray<FActiveGameplayEffectHandle> HandlesToRemove;
		// 遍历出映射中所有活动效果句柄
		for (auto HandlePar : ActiveEffectHandles)
		{
			// 如果目标能力系统组件与当前句柄对应的组件相同
			if (TargetASC == HandlePar.Value)
			{
				// 从目标组件中移除该效果
				//RemoveActiveGameplayEffect 默认为 -1，会移除所有的效果，1,只会移除1个效果
				TargetASC->RemoveActiveGameplayEffect(HandlePar.Key,1);
				// 将已移除的效果句柄添加到待移除数组中
				HandlesToRemove.Add(HandlePar.Key);
			}
		}
		// 遍历需要移除的效果句柄
		for (auto Handler : HandlesToRemove)
		{
			// 从映射中删除对应的效果句柄
			ActiveEffectHandles.FindAndRemoveChecked(Handler);
		}
		
	}
}


