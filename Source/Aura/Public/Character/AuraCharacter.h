// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "Interation/PlayerInterface.h"
#include "AuraCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class AAuraHUD;
class UNiagaraComponent;
/**
 * 
 */
UCLASS()
class AURA_API AAuraCharacter : public ACharacterBase, public IPlayerInterface
{
	GENERATED_BODY()
public:
	AAuraCharacter();
	//重写 PossessedBy ，在一个角色（Pawn）被新的控制器（Controller）接管时调用。
	virtual void PossessedBy(AController* NewController) override;
	//重写 OnRep_PlayerState ，在网络同步时，PlayerState 属性被复制（Replicated）到客户端时调用。
	virtual void OnRep_PlayerState() override;

	
	/*PlayerInterface接口*/
	virtual void AddToXP_Implementation(int32 InXP) override;
	virtual void AddToPlayerLevel_Implementation(int32 InPlayerLevel) override;
	virtual void AddToAttributePoints_Implementation(int32 InAttributePoints) override;
	virtual void AddToSpellPoints_Implementation(int32 InSpellPoints) override;
	virtual void LeveUp_Implementation() override;
	virtual int32 GetXP_Implementation() const override;
	virtual int32 GetAttributePointsReward_Implementation(int32 Level) const override;
	virtual int32 GetSpellPointsReward_Implementation(int32 Level) const override;
	virtual int32 FindLevelForXP_Implementation(int32 InXP) override;
	virtual int32 GetAttributePoints_Implementation() const override;
	virtual int32 GetSpellPoints_Implementation() const override;
	virtual void ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial = nullptr) override;
	virtual void HideMagicCircle_Implementation() override;
	virtual void SaveProgress_Implementation(const FName& CheckPointTag) override;
	/*结束PlayerInterface接口*/

	
	/**Combat 接口函数*/
	virtual int32 GetPlayerLevel_Implementation() override;
	/**结束Combat 接口函数*/

	//升级特效
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> LevelUpNiagaraComponent;

	virtual void OnRep_Stunned() override;
	virtual void OnRep_Burned() override;
	void LoadProgress();
private:
	//设置拥有者Owner Actor和Avater actor 
	virtual void InitAbilityActorInfo() override;

	//RPC,多播粒子
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLevelUpParticles();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> TopDownCameraComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom;

	
	
};


