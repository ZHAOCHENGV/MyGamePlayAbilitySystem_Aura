// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AuraAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

UAuraAttributeSet::UAuraAttributeSet()
{

	
}

void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// DOREPLIFETIME_CONDITION_NOTIFY宏 是注册属性的网络同步规则
	// COND_None: 无任何额外的同步条件，始终同步该属性
	// REPNOTIFY_Always: 每次同步该属性时，总是触发 RepNotify 函数（例如 OnRep_Health）

	//重要属性
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,Health,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,Mana,COND_None,REPNOTIFY_Always);

	//主要属性
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,Strength,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,Intelligence,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,Resilience,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,Vigor,COND_None,REPNOTIFY_Always);

	//次要属性
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,Armor,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,ArmorPenetrion,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,BlockChance,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,CriticalHitChance,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,CriticalHitDamage,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,CriticalHitResistance,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,HealthRegeneration,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,ManaRegeneration,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,MaxHealth,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet,MaxMana,COND_None,REPNOTIFY_Always);
}

void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	
	Super::PreAttributeChange(Attribute, NewValue);
	//如果Attribute值 == 血量
	if (Attribute == GetHealthAttribute())
	{
		//这里的限制是针对查询（NewValue）的值，而不是属性的最终应用值。如果游戏逻辑中有多个地方在修改属性值，而没有使用 PreAttributeChange，则可能导致不一致
		// 限制健康值在 0 和最大健康值之间
		// 注意：此处只是在查询时限制值范围，实际应用数值可能在别处修改
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
	// 数值应用时，限制 "Health" 属性的实际值在 0 和最大健康值之间
	// 此处直接对 SetHealth 和 SetMana 的结果进行限制，确保最终的属性值在有效范围内。
	// 这里是在实际应用效果后强制修正属性值，能确保逻辑一致性。
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(),0,GetMaxHealth()));
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(),0,GetMaxMana()));
	}
	
}

void UAuraAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,Armor,OldArmor);
}

void UAuraAttributeSet::OnRep_ArmorPenetrion(const FGameplayAttributeData& OldArmorPenetrion) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,ArmorPenetrion,OldArmorPenetrion);
}

void UAuraAttributeSet::OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,BlockChance,OldBlockChance);
}

void UAuraAttributeSet::OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,CriticalHitChance,OldCriticalHitChance);
}

void UAuraAttributeSet::OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,CriticalHitDamage,OldCriticalHitDamage);
}

void UAuraAttributeSet::OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,CriticalHitResistance,OldCriticalHitResistance);
}

void UAuraAttributeSet::OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,HealthRegeneration,OldHealthRegeneration);
}

void UAuraAttributeSet::OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,ManaRegeneration,OldManaRegeneration);
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
	//GAMEPLAYATTRIBUTE_REPNOTIFY宏用于处理属性更新，确保任何需要更新的引用（如UI）能正确响应属性的变化。
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,Health,OldHealth);
}


void UAuraAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,Strength,OldStrength);
}


// 当属性 Intelligence 在客户端同步时触发的函数
// 参数 OldIntelligence 表示属性更新前的旧值
void UAuraAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	// 调用 GAS 提供的宏 GAMEPLAYATTRIBUTE_REPNOTIFY
	// 自动处理属性更新后的通知逻辑（如触发回调或广播更新）
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,Intelligence,OldIntelligence);
}

void UAuraAttributeSet::OnRep_Resilience(const FGameplayAttributeData& OldResilience) const
{
	// 调用 GAS 提供的宏 GAMEPLAYATTRIBUTE_REPNOTIFY
	// 自动处理属性更新后的通知逻辑（如触发回调或广播更新）
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,Intelligence,OldResilience);
}

void UAuraAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	//处理属性在网络上变化后的通知逻辑。
	//GAMEPLAYATTRIBUTE_REPNOTIFY 宏用于处理属性更新，确保任何需要更新的引用（如UI）能正确响应属性的变化。
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet,MaxHealth,OldVigor);
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


