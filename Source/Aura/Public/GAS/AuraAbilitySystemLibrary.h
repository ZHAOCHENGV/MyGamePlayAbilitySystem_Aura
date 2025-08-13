// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AuraAbilityTypes.h"
#include "GameplayEffectTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GAS/Data/CharacterClassInfo.h"
#include "AuraAbilitySystemLibrary.generated.h"

class UAbilityInfo;
class AAuraHUD;
struct FWidgetControllerParams;
class USpellMenuWidgetController;
class UAbilitySystemComponent;
class UAttributeMenuWidgetController;
class UOverlayWidgetController;
/**
 * 
 */
UCLASS()
class AURA_API UAuraAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static bool MakeWidgetControllerParams (const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams,AAuraHUD*& OutAuraHUD);
	
	// 静态函数声明，不依赖于类的实例，而是与类本身相关联，方便全局调用.返回类型为 UOverlayWidgetController*，
	// 该函数接受一个 WorldContextObject（世界上下文对象），用于获取玩家控制器及其他相关组件。
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UOverlayWidgetController * GetOverlayWidgetController(const UObject* WorldContextObject);

	//获取 Attribute 菜单控件控制器
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UAttributeMenuWidgetController * GetAttributeMenuWidgetController(const UObject* WorldContextObject);

	//获取 Spell 菜单控件控制器
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static USpellMenuWidgetController * GetSpellMenuWidgetController(const UObject* WorldContextObject);
	
	//根据职业和等级，初始化默认属性
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|CharacterClassDefaults")
	static void InitializeDefaultAttribute(const UObject* WorldContextObject,ECharacterClass CharacterClass, float Level,UAbilitySystemComponent* ASC);

	//根据职业给予初始能力
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|CharacterClassDefaults")
	static void GiveStartupAbilities(const UObject* WorldContextObject,UAbilitySystemComponent* ASC,ECharacterClass CharacterClass);

	//获取角色类信息
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|CharacterClassDefaults")
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);


	//获取技能信息
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|Ability")
	static UAbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);
	
	//被阻止命中
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle);
	
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static float GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static float GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static float GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static FGameplayTag GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle);
	
	
	//暴击命中
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle);

	//设置阻挡命中
	//UPARAM(ref) : 告诉蓝图系统FGameplayEffectContextHandle参数应该作为引用传递
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsBlockedHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit);

	//设置暴击命中
	//UPARAM(ref) : 告诉蓝图系统FGameplayEffectContextHandle参数应该作为引用传递
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsCriticalHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit);

	//设置成功减益
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsSuccessfulDebuff(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInSuccessfulDebuff);

	//设置减益伤害
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetDebuffDamage(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InDamage);

	//设置减益持续时间
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetDebuffDuration(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InDuration);

	//设置减益频率
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetDebuffFrequency(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InFrequency);

	//设置减益类型
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetDamageType(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FGameplayTag& InDamageType);
	
	//获取半径内的玩家
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayMechanics")
	static void GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*> &ActorsToIgnore, float Radius, const FVector& SphereLocation);

	//判断是同一阵营
	//获取半径内的玩家
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayMechanics")
	static bool IsNotFriend(AActor* FirstActor,AActor* SecondActor);

	//获得职业和级别的 XP 奖励
	static int32 GetXPRewardForClassAndLevel(const UObject* WorldContextObject,ECharacterClass CharacterClass,int32 CharacterLevel) ;

	//应用伤害
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|DamageEffect")
	static FGameplayEffectContextHandle ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams);

};
