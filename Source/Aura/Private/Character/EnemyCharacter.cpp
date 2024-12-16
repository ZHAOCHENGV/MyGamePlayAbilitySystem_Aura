// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/EnemyCharacter.h"

#include "AbilitySystemComponent.h"
#include "Aura/Aura.h"
#include "GAS/AuraAbilitySystemComponent.h"
#include "GAS/AuraAttributeSet.h"

AEnemyCharacter::AEnemyCharacter()
{
	//设置网格体碰撞对可视性阻挡
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
	//初始化AbilitySystemComponent组件
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	//设置AbilitySystemComponent组件为可复制
	AbilitySystemComponent->SetIsReplicated(true);
	//设置AbilitySystemComponent组件的复制模式
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	//创建AttributeSet属性集合
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");

	
}

void AEnemyCharacter::HigHlihtActor()
{
	
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->SetRenderCustomDepth(true);
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}

void AEnemyCharacter::UnHigHlightActor()
{
	
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

int32 AEnemyCharacter::GetPlayerLevel()
{
	return Level;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitAbilityActorInfo();
}

void AEnemyCharacter::InitAbilityActorInfo()
{
	//设置拥有者Owner Actor和Avater actor 为自身
	AbilitySystemComponent->InitAbilityActorInfo(this,this);
	//获取类型为UAuraAbilitySystemComponent技能组件，并且初始化技能属性集
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
}
 