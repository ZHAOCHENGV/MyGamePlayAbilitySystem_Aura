// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AuraGamePlayTags.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "GameplayTagContainer.h"
#include "SpellMenuWidgetController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSpellGlobeSelectedSignature,bool,bSpendPointsButtonEnable,bool,bEquipButtonEnabled,FString,DescriptionString,FString,NextLevelDescriptionString);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitForEquipSelectionSignature, const FGameplayTag&, AbilityType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpellGlobeReassignedSignature, const FGameplayTag&, AbilityTag);

struct FSelectedAbility
{
	FGameplayTag AbilityTag = FGameplayTag();
	FGameplayTag Status = FGameplayTag();
};
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API USpellMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()
public:
	//广播初始值
	virtual void BroadcastInitialValues() override;
	
	//将回调函数绑定到依赖项
	virtual void BindCallbacksToDependencies() override;

	//UPROPERTY(BlueprintAssignable) 修饰意味着此委托可以在蓝图中直接绑定响应函数
	UPROPERTY(BlueprintAssignable)
	//技能点改变委托
	FOnPlayerStatChangedSignature SpellPointsChanged;

	//等待装备技能委托
	UPROPERTY(BlueprintAssignable)
	FWaitForEquipSelectionSignature WaitForEquipDelegate;

	//停止等待装备技能委托
	UPROPERTY(BlueprintAssignable)
	FWaitForEquipSelectionSignature StopWaitForEquipDelegate;
	
	//选择的技能委托
	UPROPERTY(BlueprintAssignable)
	FSpellGlobeSelectedSignature SpellGlobeSelectedDelegate;

	//法术球分配委托
	UPROPERTY(BlueprintAssignable)
	FSpellGlobeReassignedSignature SpellGlobeReassignedDelegate;
	
	//技能已选中
	UFUNCTION(BlueprintCallable)
	void SpellGlobeSelected(const FGameplayTag& AbilityTag);

	//按下消耗技能点按钮
	UFUNCTION(BlueprintCallable)
	void SpendPointButtonPressed();

	//取消技能选择
	UFUNCTION(BlueprintCallable)
	void GlobeDeselect();

	//装备按钮按下
	UFUNCTION(BlueprintCallable)
	void EquipButtonPressed();

	//按下装备栏中的技能球逻辑
	UFUNCTION(BlueprintCallable)
	void SpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType);

	//装备能力
	void OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status,const FGameplayTag& Slot, const FGameplayTag& PreviousSlot);
	
private:
	//启用按钮
	static void ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bShouldEnableSpellPointsButton, bool& bShouldEnableEquipButton);	
	//所选能力
	FSelectedAbility SelectedAbility = {FAuraGamePlayTags::Get().Abilities_None , FAuraGamePlayTags::Get().Abilities_Status_Locked};
	//当前法术点
	int32 CurrentSpellPoints = 0;
	//等待装备选择?
	bool bWaitingForEquipSelection = false;
	//选择的技能插槽
	FGameplayTag SelectedSlot;


};
