// Fill out your copyright notice in the Description page of Project Settings.



#include "UI/WidgetController/OverlayWidgetController.h"

#include "GAS/AuraAttributeSet.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	UAuraAttributeSet * AuraAttributeSet = Cast<UAuraAttributeSet>(AttributeSet);
	//广播初始值
	OnHealthChangeds.Broadcast(AuraAttributeSet->GetHealth());
	OnMaxHealthChangeds.Broadcast(AuraAttributeSet->GetMaxHealth());

}
