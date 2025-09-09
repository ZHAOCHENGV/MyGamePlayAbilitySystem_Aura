// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AuraAbilityTypes.h"
#include "GameFramework/Actor.h"
#include "AuraProjectile.generated.h"



class UNiagaraSystem;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class AURA_API AAuraProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	AAuraProjectile();
	// 可见的投射物运动组件，用于控制投射物的移动逻辑
	// VisibleAnywhere：表示该属性可见，但在编辑器中不可编辑
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent>  ProjectileMovement;

	// 蓝图可读写的属性，用于传递投射物的伤害效果
	// BlueprintReadWrite：表示该属性在蓝图中可读写
	// meta = (ExposeOnSpawn = true)：允许该属性在蓝图中实例化时（例如通过 SpawnActor 方法）被设置
	UPROPERTY(BlueprintReadWrite,meta = (ExposeOnSpawn = true))
	FDamageEffectParams DamageEffectParams;
	//球体组件
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	TObjectPtr<USphereComponent> Sphere;

	//归位目标的场景组件
	UPROPERTY()
	TObjectPtr<USceneComponent> HomingTargetSceneComponent;

protected:
	virtual void BeginPlay() override;
	UFUNCTION(BlueprintCallable)
	virtual void OnHit();
	virtual void Destroyed() override;
	bool IsValidOverlap(AActor* OtherActor);
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	//击中？
	bool bHit = false;
	
	//循环音效组件
	UPROPERTY()
	TObjectPtr<UAudioComponent> LoopingSoundComponent;

private:
	//生命周期
	UPROPERTY(EditDefaultsOnly)
	float LifeSpan = 15.f;



	
	//击中特效
	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> ImpactEffect;
	//击中音效
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> ImpactSound;
	//循环声音
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> LoopingSound;
	
};
