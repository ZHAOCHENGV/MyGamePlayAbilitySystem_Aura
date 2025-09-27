// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "AuraEffectActor.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

//游戏效果应用策略枚举
UENUM(BlueprintType)
enum class EEffectApplicationPolicy : uint8
{
	ApplyOnOverlap,
	ApplyOnEndOverlap,
	DotNotApply
};

//效果移除政策枚举
UENUM(BlueprintType)
enum class EEffectRemovalPolicy : uint8
{
	RemoveOnEndOverlap,
	DoNotRemove

};
UCLASS()
class AURA_API AAuraEffectActor : public AActor
{
	GENERATED_BODY()
	
public:	

	AAuraEffectActor();
	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;
	//计算位置
	UPROPERTY(BlueprintReadWrite)
	FVector CalculatedLocation;
	//计算旋转
	UPROPERTY(BlueprintReadWrite)
	FRotator CalculatedRotation;
	//旋转
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Pickup Movement")
	bool bRotates = false;
	//旋转速率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	float RotationRate = 45.f;
	//开启正弦运动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	bool bSinusoidalMovement = false;
	//正弦振幅
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	float SineAmplitude = 1.f;
	//正弦周期常数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	float SinePeriodConstant = 1.f;
	//初始位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	FVector InitialLocation;
	
	UFUNCTION(BlueprintCallable)
	void StartSinusoidalMovement();

	UFUNCTION(BlueprintCallable)
	void StartRotation();

	
	//应用效果到目标
	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(AActor * TargetActor,TSubclassOf<UGameplayEffect> GameplayEffectClass);

	//重叠时
	UFUNCTION(BlueprintCallable)
	void OnOverlap(AActor * TargetActor);

	//结束重叠时
	UFUNCTION(BlueprintCallable)
	void OnEndOverlap(AActor * TargetActor);

	
	//效果应用时销毁？
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Applied Effects")
	bool bDestroyOnEffectApplication = false;

	//对敌人施加效果？
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Applied Effects")
	bool bApplyEffectsToEnemies = false;
	

	//创建映射容器  FActiveGameplayEffectHandle是key，UAbilitySystemComponent是value
	//#include "GameplayEffectTypes.h"  解决FActiveGameplayEffectHandle 报错
	TMap<FActiveGameplayEffectHandle,UAbilitySystemComponent*> ActiveEffectHandles;
	
	//即时游戏效果
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Applied Effects")
	TSubclassOf<UGameplayEffect> InstantGameplayEffectClass;
	//应用效果策略
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Applied Effects")
	EEffectApplicationPolicy InstantEffectApplicationPolicy = EEffectApplicationPolicy::DotNotApply;

	//持续游戏效果
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Applied Effects")
	TSubclassOf<UGameplayEffect> DurationGameplayEffectClass;
	//应用效果策略
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Applied Effects")
	EEffectApplicationPolicy DurationEffectApplicationPolicy = EEffectApplicationPolicy::DotNotApply;

	//永久游戏效果
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Applied Effects")
	TSubclassOf<UGameplayEffect> InfiniteGameplayEffectClass;
	//应用效果策略
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Applied Effects")
	EEffectApplicationPolicy InfiniteEffectApplicationPolicy = EEffectApplicationPolicy::DotNotApply;
	//移除效果策略
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Applied Effects")
	EEffectRemovalPolicy InfiniteEEffectRemovalPolicy = EEffectRemovalPolicy::RemoveOnEndOverlap;
	
	//效果等级
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Applied Effects")
	float ActorLevel = 1.0f;

private:
	float RunningTime = 0.f;

	void ItemMovement(float DeltaTime);
};
