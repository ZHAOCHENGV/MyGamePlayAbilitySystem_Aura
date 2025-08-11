// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerState.h"

#include "GAS/AuraAbilitySystemComponent.h"
#include "GAS/AuraAttributeSet.h"
#include "Net/UnrealNetwork.h"

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

void AAuraPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//Doreplifetime  注册Level属性的同步规则
	DOREPLIFETIME(AAuraPlayerState,Level);
	DOREPLIFETIME(AAuraPlayerState,Xp);
	DOREPLIFETIME(AAuraPlayerState,AttributePoints);
	DOREPLIFETIME(AAuraPlayerState,SpellPoints);

}


UAbilitySystemComponent* AAuraPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// 增加经验值实现
void AAuraPlayerState::AddToXP(int32 InXP)
{
	Xp += InXP;
	// 广播经验变更委托（客户端通过RepNotify接收）
	OnXPChangedDelegate.Broadcast(Xp);
}

// 提升等级实现
void AAuraPlayerState::AddToLevel(int32 InLevel)
{
	Level += InLevel;
	// 广播等级变更委托
	OnLevelChangedDelegate.Broadcast(Level);
}



void AAuraPlayerState::SetXP(int32 InXP)
{
	Xp = InXP;
	OnXPChangedDelegate.Broadcast(Xp);
}

void AAuraPlayerState::SetLevel(int32 InLevel)
{
	Level = InLevel;
	OnLevelChangedDelegate.Broadcast(Level);
}


void AAuraPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
}

// 等级同步回调（客户端执行）
void AAuraPlayerState::OnRep_Level(int32 OldLevel)
{
	// 可在此处触发升级特效等客户端专属逻辑
	OnLevelChangedDelegate.Broadcast(Level);
}

// 经验值同步回调（客户端执行）
void AAuraPlayerState::OnRep_Xp(int32 OldXp)
{
	// 客户端接收到新经验值时触发UI更新
	OnXPChangedDelegate.Broadcast(Xp);
}

void AAuraPlayerState::OnRep_AttributePoints(int32 OldAttributePoints)
{
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
	
}

void AAuraPlayerState::OnRep_SpellPoints(int32 OldSpellPoints)
{
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}

void AAuraPlayerState::AddToAttributePoints(int32 InAttributePoints)
{
	AttributePoints += InAttributePoints;
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void AAuraPlayerState::AddToSpellPoints(int32 InSpellPoints)
{
	SpellPoints += InSpellPoints;
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}