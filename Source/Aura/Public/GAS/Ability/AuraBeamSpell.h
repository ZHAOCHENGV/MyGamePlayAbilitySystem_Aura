// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/AuraDamageGameplayAbility.h"
#include "AuraBeamSpell.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraBeamSpell : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()

public:

	//存储鼠标信息
	UFUNCTION(BlueprintCallable)
	void StoreMouseDataInfo(const FHitResult& HitResult);

	//存储变量
	UFUNCTION(BlueprintCallable)
	void StoreOwnerVariables();

	//检测第一个目标
	UFUNCTION(BlueprintCallable)
	void TraceFirstTarget(const FVector& BeamTargetLocation);

	//存储其他目标
	UFUNCTION(BlueprintCallable)
	void StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets);

	
	UFUNCTION(BlueprintImplementableEvent)
	void PirmaryTargetDied(AActor* DeadActor);

	UFUNCTION(BlueprintImplementableEvent)
	void AdditionalTargetDied(AActor* DeadActor);

	
protected:

	//光标命中位置
	UPROPERTY(BlueprintReadWrite, Category = "Beam")
	FVector MouseHitLocation;

	//光标命中Actor
	UPROPERTY(BlueprintReadWrite, Category = "Beam")
	TObjectPtr<AActor> MouseHitActor;

	//所有者玩家控制器
	UPROPERTY(BlueprintReadWrite, Category = "Beam")
	TObjectPtr<APlayerController> OwnerPlayerController;

	
	//所有者
	UPROPERTY(BlueprintReadWrite, Category = "Beam")
	TObjectPtr<ACharacter> OwnerCharacter;

	
	UPROPERTY(EditDefaultsOnly, Category = "Beam")
	int32 MaxNumShockTargets = 5;
};
