// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "AuraAttributeSet.generated.h"





//这段宏定义通过组合多个功能性宏，快速为指定类和属性生成常见的访问器函数。

//GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) // 生成 Getter 方法以获取属性对象
//GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)           // 生成 Getter 方法以获取属性的当前值
//GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)               // 生成 Setter 方法以设置属性的值
//GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)                 // 生成初始化方法以设置初始值
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)           



USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()
	
	FEffectProperties() {} //析构函数

	//效果上下文
	FGameplayEffectContextHandle EffectContextHandle;

	//源能力系统组件
	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;

	//源化身演员
	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;

	//源控制器
	UPROPERTY()
	AController* SourceController = nullptr;

	//源角色
	UPROPERTY()
	ACharacter* SourceCharacter = nullptr;

	//目标能力组件
	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	//目标化身演员
	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;

	//目标控制器
	UPROPERTY()
	AController* TargetController = nullptr;

	//目标角色
	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
	
};

//是一个非常长的类型名，直接使用会显得代码繁琐且不直观。使用 typedef 为这个复杂类型起一个简短别名 FAttributeFuncPtr
//typedef TBaseStaticDelegateInstance<FGameplayAttribute(),FDefaultDelegateUserPolicy>::FFuncPtr FAttributeFuncPtr;
//通用模板，可以接收任意类型 T
//using 是现代 C++ 的类型别名定义方式（比 typedef 更强大，更容易处理复杂模板类型）
//typename 用于显式指定 TBaseStaticDelegateInstance<FGameplayAttribute(), FDefaultDelegateUserPolicy>::FFuncPtr 是一个类型（避免歧义）。
//T 函数签名
//让代码在需要使用不同静态函数类型时更通用、更灵活、不仅可以绑定 FGameplayAttribute() 这样的返回值类型，还可以绑定其他函数签名类型。
//typedef 特定于 FGameplayAttribute() 函数签名，而 TStaticFuncPtr 是通用的，可以适用于任何函数签名。
template<class T>
using TStaticFuncPtr =typename  TBaseStaticDelegateInstance<T,FDefaultDelegateUserPolicy>::FFuncPtr;

/**
 * 
 */
UCLASS()
class AURA_API UAuraAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	//构造函数
	UAuraAttributeSet();


	//这是一个虚拟函数，用于定义哪些属性需要在网络中进行复制（replication）。
	//通过重写这个函数，开发者可以指定哪些属性在网络同步时需要被管理。
	// 是 Unreal Engine 用于存储和管理需要复制的属性的数组。
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//属性更改函数，属性正式更改之前会调用这个函数，因此可以在此函数中限制值得数值
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	//Gameplay Effect执行后处理函数,该函数主要用于在Gameplay Effect（游戏效果）执行完成后，做一些特定的后处理操作
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	/*
	 *	旧版代码TMap<FGameplayTag,TBaseStaticDelegateInstance<FGameplayAttribute(),FDefaultDelegateUserPolicy>::FFuncPtr> TagsToAttributes;
	 *	TBaseStaticDelegateInstance是 Unreal Engine 中的一个模板类，用于创建静态委托（不依赖于对象实例的委托）。
	 *	FGameplayAttribute() 是模板类中的类型参数，表示委托函数的签名将接受一个 FGameplayAttribute 类型的参数
	 *	FGameplayAttribute 可能表示一个游戏属性（例如力量、智力等）。
	 *	FDefaultDelegateUserPolicy 是默认的委托用户策略，通常控制如何管理委托的绑定和解绑。
	 *	FFuncPtr 是 TBaseStaticDelegateInstance 的一个成员类型，它是一个指向函数的指针，指向与 FGameplayAttribute 类型兼容的函数。
	 *	因此，TBaseStaticDelegateInstance<FGameplayAttribute(), FDefaultDelegateUserPolicy>::FFuncPtr 是一个指向
	 *	处理 FGameplayAttribute 类型参数的静态函数的指针。
	 *	这意味着 TagsToAttributes 将会存储一组标签（FGameplayTag）和它们对应的函数指针（FFuncPtr）。每个函数
	 *	指针可以指向一个与特定游戏属性相关的函数，例如获取“力量”属性的函数。
	 */
	//	
	TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>> TagsToAttributes;
	
	/*
	 * 主要属性
	 */
	//创建力量值
	// 声明一个只读的属性，支持蓝图和网络同步，标记为“Primary Attributes”分类
	// ReplicatedUsing=OnRep_Health 表示该变量在客户端更新时会调用 OnRep_Strength 函数
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_Strength,Category = "Primary Attributes")
	FGameplayAttributeData Strength;
	// 使用宏为 Strength 属性生成访问器函数
	// ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Strength) 会生成如下两个函数：
	// 1. GetStrength(): 用于获取 Strength 的值
	// 2. SetStrength(): 用于设置 Strength 的值
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,Strength);
	//创建智力
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_Intelligence,Category = "Primary Attributes")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,Intelligence);
	//创建韧性
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_Resilience,Category = "Primary Attributes")
	FGameplayAttributeData Resilience;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,Resilience);
	//活力
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_Vigor,Category = "Primary Attributes")
	FGameplayAttributeData Vigor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,Vigor);
	
	//在客户端力量值更新时会调用 OnRep_Strength 函数
	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldStrength) const;
	
	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const;
	
	UFUNCTION()
	void OnRep_Resilience(const FGameplayAttributeData& OldResilience) const;
	
	UFUNCTION()
	void OnRep_Vigor(const FGameplayAttributeData& OldVigor) const;

	

	/*
	 * 重要属性
	 */
	//创建健康值 ,ReplicatedUsing = OnRep_Health 指定当该属性在网络上被复制时，它会调用 OnRep_Health 方法来通知任何相关的对象或系统。
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_Health,Category = "Vital Attributes")
	FGameplayAttributeData Health;
	//属性器访问。
	//这个宏会自动创建初始化，get,set，等方法
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,Health);
	
	//创建魔法值 ,ReplicatedUsing = OnRep_Health 指定当该属性在网络上被复制时，它会调用 OnRep_Health 方法来通知任何相关的对象或系统。
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_Mana,Category = "Vital Attributes")
	FGameplayAttributeData Mana;
	//这个宏会自动创建初始化，get,set，等方法
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,Mana);
	

	//Rep_Health 方法会在 Health 属性在网络上复制并被修改时调用。
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;
	
	//Rep_Health 方法会在 Mana 属性在网络上复制并被修改时调用。
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;




	
	/*
	 * 次要属性
	 */
	//护甲，派生自Resilience，减少伤害提高格挡伤害
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_Armor,Category = "Secondary Attributes")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,Armor);
	//护甲穿透，派生自Resilience，忽略敌方护甲百分比，增加暴击率
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_ArmorPenetrion,Category = "Secondary Attributes")
	FGameplayAttributeData ArmorPenetrion;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,ArmorPenetrion);
	//阻挡几率，派生自Armor，有几率把伤害减半
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_BlockChance,Category = "Secondary Attributes")
	FGameplayAttributeData BlockChance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,BlockChance);
	//暴击率，派生自ArmorPenetration，几率使伤害和暴击加成加倍
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_CriticalHitChance,Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitChance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,CriticalHitChance);
	//暴击伤害，派生自ArmorPenetration，暴击增加额外伤害
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_CriticalHitDamage,Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitDamage;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,CriticalHitDamage);
	//暴击抗性，派生自Armor，降低敌人暴击的几率
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_CriticalHitResistance,Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,CriticalHitResistance);
	//生命恢复，派生自Vigor，每秒恢复的生命值
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_HealthRegeneration,Category = "Secondary Attributes")
	FGameplayAttributeData HealthRegeneration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,HealthRegeneration);
	//法力恢复，派生自Intelligence，每秒恢复的法力值
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_ManaRegeneration,Category = "Secondary Attributes")
	FGameplayAttributeData ManaRegeneration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,ManaRegeneration);
	//创建最大健康值 ,ReplicatedUsing = OnRep_MaxHealth,指定当该属性在网络上被复制时，它会调用 OnRep_Health 方法来通知任何相关的对象或系统。
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_MaxHealth,Category = "Secondary Attributes")
	FGameplayAttributeData MaxHealth;
	//这个宏会自动创建初始化，get,set，等方法
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,MaxHealth);
	//创建最大魔法值 ,ReplicatedUsing = OnRep_MaxHealth,指定当该属性在网络上被复制时，它会调用 OnRep_Health 方法来通知任何相关的对象或系统。
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_MaxMana,Category = "Secondary Attributes")
	FGameplayAttributeData MaxMana;
	//这个宏会自动创建初始化，get,set，等方法
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,MaxMana);
	
	/*
	 * 伤害类型抗性属性
	 */
	//火焰类型伤害抗性属性 
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_FireResistance,Category = "Secondary Attributes")
	FGameplayAttributeData FireResistance;
	//这个宏会自动创建初始化，get,set，等方法
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,FireResistance);

	//闪电类型伤害抗性属性 
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_LightningResistance,Category = "Secondary Attributes")
	FGameplayAttributeData LightningResistance;
	//这个宏会自动创建初始化，get,set，等方法
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,LightningResistance);

	//奥术类型伤害抗性属性 
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_ArcaneResistance,Category = "Secondary Attributes")
	FGameplayAttributeData ArcaneResistance;
	//这个宏会自动创建初始化，get,set，等方法
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,ArcaneResistance);

	//物理类型伤害抗性属性 
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_PhysicalResistance,Category = "Secondary Attributes")
	FGameplayAttributeData PhysicalResistance;
	//这个宏会自动创建初始化，get,set，等方法
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,PhysicalResistance);

	/* 
	 * 元属性Meta Attributes
	 */
	//来袭伤害
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData IncomingDamage;
	//这个宏会自动创建初始化，get,set，等方法
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,IncomingDamage);

	//传入经验值
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData IncomingXP;
	//这个宏会自动创建初始化，get,set，等方法
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,IncomingXP);

	

	
	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor) const;
	UFUNCTION()
	void OnRep_ArmorPenetrion(const FGameplayAttributeData& OldArmorPenetrion) const;
	UFUNCTION()
	void OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const;
	UFUNCTION()
	void OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const;
	UFUNCTION()
	void OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const;
	UFUNCTION()
	void OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const;
	UFUNCTION()
	void OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const;
	UFUNCTION()
	void OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const;
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;
	UFUNCTION()
	void OnRep_FireResistance(const FGameplayAttributeData& OldFireResistance) const;
	UFUNCTION()
	void OnRep_LightningResistance(const FGameplayAttributeData& OldLightningResistance) const;
	UFUNCTION()
	void OnRep_ArcaneResistance(const FGameplayAttributeData& OldArcaneResistance) const;
	UFUNCTION()
	void OnRep_PhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance) const;
	
	

private:
	
	//设置效果属性
	void SetEffectProperties(const struct FGameplayEffectModCallbackData& Data,FEffectProperties & Props)const;

	//显示伤害文本
	void ShowFloatingText(const FEffectProperties& Props, float Damage, bool bBlockedHit, bool bCriticalHit) const;

	//发送经验值事件
	void SendXPEvent(const FEffectProperties& Props);

	//到达最大生命值
	bool bTopOffHealth = false;
	//到达最大魔法值
	bool bTopOffMana = false;
};
