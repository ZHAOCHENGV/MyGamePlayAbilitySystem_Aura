// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AuraAbilityTypes.h"
#include "GAS/Ability/AuraGameplayAbility.h"
#include "Interation/CombatInterface.h"
#include "AuraDamageGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraDamageGameplayAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()
public:

	//造成损害
	UFUNCTION(BlueprintCallable)
	void CauseDamage(AActor* TargetActor);

	//从类默认值创建伤害效果参数
	UFUNCTION(BlueprintPure)
	FDamageEffectParams MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor = nullptr) const;

	//获取根据等级的上海数值
	UFUNCTION(BlueprintPure)
	float GetDamageAtLevel() const;
protected:
	//伤害游戏效果
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	//伤害类型
	UPROPERTY(EditDefaultsOnly,  Category = "Damage")
	FGameplayTag DamageType;
	//伤害值
	UPROPERTY(EditDefaultsOnly,  Category = "Damage")
	FScalableFloat	Damage;

	//减益几率
	UPROPERTY(EditDefaultsOnly,  Category = "Damage", meta=(DisplayName="Debuff几率"))
	float DebuffChance = 20.f;

	//减益效果伤害值
	UPROPERTY(EditDefaultsOnly,  Category = "Damage", meta=(DisplayName="Debuff伤害"))
	float DebuffDamage = 5.f;

	//减益效果频率
	UPROPERTY(EditDefaultsOnly,  Category = "Damage", meta=(DisplayName="Debuff频率"))
	float DebuffFrequency = 1.f;

	//减益效果持续时间
	UPROPERTY(EditDefaultsOnly,  Category = "Damage", meta=(DisplayName="Debuff持续时间"))
	float DebuffDuration = 5.f;

	//死亡冲量强度
	UPROPERTY(EditDefaultsOnly,  Category = "Damage", meta=(DisplayName="死亡冲量强度"))
	float DeathImpulseMagnitude = 1000.f;

	//击退强度
	UPROPERTY(EditDefaultsOnly,  Category = "Damage", meta=(DisplayName="技能击退强度"))
	float KnockBackForceMagnitude = 1000.f;

	//击退几率
	UPROPERTY(EditDefaultsOnly,  Category = "Damage", meta=(DisplayName="技能击退几率"))
	float KnockBackChance = 0.f;

	
	//从数组中随机获取攻击的蒙太奇
	UFUNCTION(BlueprintPure)
	FTaggedMontage GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& TaggedMontages) const;

};
