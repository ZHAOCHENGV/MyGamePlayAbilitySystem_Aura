#pragma once

#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "AuraAbilityTypes.generated.h"


/**
 * 伤害效果参数结构体（蓝图可见）
 * 
 * 功能说明：
 * 统一封装伤害计算和应用所需的所有参数
 * 用于在技能系统间传递复杂的伤害配置
 */
USTRUCT(BlueprintType)
struct FDamageEffectParams
{
	GENERATED_BODY()

	// 世界上下文对象（必需）
	// 用途：生成视觉效果、播放音效、获取世界信息
	// 注意：必须有效，否则无法生成视觉特效
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UObject> WorldContextObject = nullptr;

	// 伤害效果类（必需）
	// 定义伤害如何计算和应用的GameplayEffect
	// 示例：烧伤、冰冻、中毒等效果
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass = nullptr;

	// 来源能力系统组件（必需）
	// 造成伤害的实体（玩家、敌人等）的能力系统
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent;

	// 目标能力系统组件（必需）
	// 承受伤害的实体的能力系统
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;

	// 基础伤害值
	// 实际伤害可能受属性影响而变动
	UPROPERTY(BlueprintReadWrite)
	float BaseDamage = 0.f;

	// 技能等级
	// 用于伤害曲线缩放和效果强度
	UPROPERTY(BlueprintReadWrite)
	float AbilityLevel = 1.f;

	// 伤害类型标签
	// 标识伤害属性（如火、冰、物理等）
	// 用于抗性计算和视觉效果选择
	FGameplayTag DamageType = FGameplayTag();
    
	// 减益触发概率（0.0-1.0）
	// 示例：0.3表示30%概率施加减益
	UPROPERTY(BlueprintReadWrite)
	float DebuffChance = 0.f;

	// 减益效果伤害值
	// 减益状态期间每次触发的伤害
	UPROPERTY(BlueprintReadWrite)
	float DebuffDamage = 0.f;

	// 减益持续时间（秒）
	// 减益状态持续的总时长
	UPROPERTY(BlueprintReadWrite)
	float DebuffDuration = 0.f;

	// 减益触发频率（秒）
	// 减益效果每次触发的间隔时间
	UPROPERTY(BlueprintReadWrite)
	float DebuffFrequency = 0.f;
	
	//死亡冲量强度
	UPROPERTY(BlueprintReadWrite)
	float DeathImpulseMagnitude = 0.f;

	//死亡脉冲位置
	UPROPERTY(BlueprintReadWrite)
	FVector DeathImpulse = FVector::ZeroVector;

	//击退强度
	UPROPERTY(BlueprintReadWrite)
	float KnockBackForceMagnitude = 0.f;

	//击退几率
	UPROPERTY(BlueprintReadWrite)
	float KnockBackChance = 0.f;
	
	//击退位置
	UPROPERTY(BlueprintReadWrite)
	FVector KnockBackForce = FVector::ZeroVector;

	//径向损伤
	UPROPERTY(BlueprintReadWrite)
	bool bIsRadialDamage = false;

	//径向损伤内半径
	UPROPERTY(BlueprintReadWrite)
	float RadialDamageInnerRadius = 0.f;


	//径向损伤外半径
	UPROPERTY(BlueprintReadWrite)
	float RadialDamageOuterRadius = 0.f;

	//径向损伤原点
	UPROPERTY(BlueprintReadWrite)
	FVector RadialDamageOrigin = FVector::ZeroVector;
	

	
	
};






USTRUCT(BlueprintType)
struct FAuraGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()
	
public:
	bool IsCriticalHit() const { return bIsCriticalHit; }
	bool IsBlockedHit() const { return bIsBlockedHit; }
	bool IsSuccessfulDebuff() const { return bIsSuccessfulDebuff; }
	float GetDebuffDamage() const {return DebuffDamage;}
	float GetDebuffDuration() const {return DebuffDuration;}
	float GetDebuffFrequency() const {return DebuffFrequency;}
	TSharedPtr<FGameplayTag> GetDamageType() const {return DamageType;}
	FVector GetDeathImpulse() const {return DeathImpulse;}
	FVector GetKnockBackForce() const {return KnockBackForce;}
	bool IsRadialDamage() const { return bIsRadialDamage; }
	float GetRadialDamageOuterRadius() const {return RadialDamageOuterRadius;}
	float GetRadialDamageInnerRadius() const {return RadialDamageInnerRadius;}
	FVector GetRadialDamageOrigin() const {return RadialDamageOrigin;}

	void SetIsCriticalHit(bool bInIsCriticalHit) { bIsCriticalHit = bInIsCriticalHit; }
	void SetIsBlockedHit(bool bInIsBlockedHit){ bIsBlockedHit = bInIsBlockedHit; }
	void SetIsSuccessfulDebuff(bool bInIsDebuff) {bIsSuccessfulDebuff = bInIsDebuff;}
	void SetDebuffDamage(float InDamage){DebuffDamage = InDamage;}
	void SetDebuffDuration(float InDuration){DebuffDuration = InDuration;}
	void SetDebuffFrequency(float InFrequency){DebuffFrequency = InFrequency;}
	void SetDamageType(TSharedPtr<FGameplayTag> InDamageType){DamageType = InDamageType;}
	void SetDeathImpulse(const FVector& InImpulse){DeathImpulse = InImpulse;}
	void SetKnockBackForce(const FVector& InForce){KnockBackForce = InForce;}
	void SetIsRadialDamage(bool bInIsRadialDamage){bIsRadialDamage = bInIsRadialDamage;}
	void SetRadialDamageInnerRadius(float InRadialDamageInnerRadius){RadialDamageInnerRadius = InRadialDamageInnerRadius;}
	void SetRadialDamageOuterRadius(float InRadialDamageOuterRadius){RadialDamageOuterRadius = InRadialDamageOuterRadius;}
	void SetRadialDamageOrigin(const FVector& InRadialDamageOrigin){RadialDamageOrigin = InRadialDamageOrigin;}
	
	/** 返回用于序列化的实际结构体，子类必须覆盖 this！ */
	virtual UScriptStruct* GetScriptStruct() const
	{
		return StaticStruct();
	}

	/* 自定义序列化，子类必须覆盖此
	 * FArchive& Ar : 数据流对象，负责读写序列化后的二进制数据。 序列化（如服务器发送数据）时，Ar处于写入模式，将数据打包到流中。反序列化（如客户端接收数据）时，Ar处于读取模式，从流中解包数据。
	 * class UPackageMap* Map : 管理对象与网络唯一标识符（如NetGUID）的映射，解决网络对象引用问题。序列化时，将UObject*转换为NetGUID以便传输。反序列化时，通过NetGUID查找本地对应的UObject*。
	 * bool& bOutSuccess : 输出参数，表示序列化/反序列化是否成功。
	 */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);


	// 虚函数 Duplicate() 用于复制一个 FAuraGameplayEffectContext 对象
	virtual FAuraGameplayEffectContext* Duplicate() const
	{
		// 创建一个新的 FAuraGameplayEffectContext 对象，使用 new 动态分配内存
		FAuraGameplayEffectContext* NewContext = new FAuraGameplayEffectContext();
		// 将当前对象的所有数据复制到新创建的对象中
		// 这里使用赋值操作符（operator=），假设 FAuraGameplayEffectContext 已经实现了合适的复制逻辑
		*NewContext = *this;
		// 如果当前上下文中存在有效的 HitResult（碰撞结果）
		if (GetHitResult())
		{
			// 将当前 HitResult 的数据添加到新对象中
			// AddHitResult 的第二个参数通常表示是否要进行深拷贝（true 表示深拷贝，复制数据而非指针引用）
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		// 返回新复制的上下文对象
		return NewContext;
	}

protected:
	
	//是否被格挡
	UPROPERTY()
	bool bIsBlockedHit = false;
	//是否暴击
	UPROPERTY()
	bool bIsCriticalHit = false;

	//是否有Debuff
	UPROPERTY()
	bool bIsSuccessfulDebuff = false;
	//Debuff伤害
	UPROPERTY()
	float DebuffDamage = 0.f;
	//Debuff持续时间
	UPROPERTY()
	float DebuffDuration = 0.f;
	//Debuff频率
	UPROPERTY()
	float DebuffFrequency = 0.f;
	//伤害类型
	TSharedPtr<FGameplayTag> DamageType;
	//死亡脉冲
	UPROPERTY()
	FVector DeathImpulse = FVector::ZeroVector;
	//击退位置
	UPROPERTY()
	FVector KnockBackForce = FVector::ZeroVector;

	//径向损伤
	UPROPERTY()
	bool bIsRadialDamage = false;
	//径向损伤内半径
	UPROPERTY()
	float RadialDamageInnerRadius = 0.f;
	//径向损伤外半径
	UPROPERTY()
	float RadialDamageOuterRadius = 0.f;
	//径向损伤原点
	UPROPERTY()
	FVector RadialDamageOrigin = FVector::ZeroVector;
	
};
	/*
	 * TStructOpsTypeTraits :  模板结构用于告诉引擎如何处理一个结构体（或类）的特殊操作。这些特殊操作包括网络序列化、复制、比较等
	 * TStructOpsTypeTraits<FAuraGameplayEffectContext> : 这是对自定义结构体 FAuraGameplayEffectContext 的特化（即为该结构体定制特定操作属性）,
	 * 特化后的结构体用于描述引擎在处理 FAuraGameplayEffectContext 时应当采用哪些特殊行为。
	 * StructOpsTypeTraitsBase2<FAuraGameplayEffectContext> : 这是一个基础模板结构体，提供了 FAuraGameplayEffectContext 的默认操作行为，
	 * 通过继承自 TStructOpsTypeTraitsBase2，我们可以保留默认行为，并在此基础上进行修改或扩展。
	 */
template<>
struct TStructOpsTypeTraits<FAuraGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FAuraGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,// 表示该结构体提供了自定义的 NetSerialize 函数，用于网络序列化
		WithCopy = true// 表示该结构体支持复制操作（拷贝构造/赋值操作）
	};
};

