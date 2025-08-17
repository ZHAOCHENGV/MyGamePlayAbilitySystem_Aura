// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/AuraProjectileSpell.h"
#include "AuraFireBolt.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraFireBolt : public UAuraProjectileSpell
{
	GENERATED_BODY()
public:
	
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;
	UFUNCTION(BlueprintCallable)
	void SpawnProjectiles(const FVector& ProjectileTargetLocation,const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride, AActor* HomingTarget);

protected:
	
	//射弹扩散
	UPROPERTY(EditDefaultsOnly, Category="FireBolt")
	float ProjectileSpread = 90.f;

	//最大发射数量
	UPROPERTY(EditDefaultsOnly, Category="FireBolt")
	int32 MaxNumProjectiles = 5;

	//归位加速度最小值
	UPROPERTY(EditDefaultsOnly, Category="FireBolt")
	float HomingAccelerationMin = 1600.f;

	//归位加速度最大值
	UPROPERTY(EditDefaultsOnly, Category="FireBolt")
	float HomingAccelerationMax = 3200.f;

	//是否跟踪
	UPROPERTY(EditDefaultsOnly, Category="FireBolt")
	bool bLaunchHomingProjectiles = true;
};
