// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/AuraHUD.h"
#include "UI/Widget/AuraUserWidget.h"

void AAuraHUD::BeginPlay()
{
	Super::BeginPlay();
	//创建控件
	UUserWidget * Widget = CreateWidget<UUserWidget>(GetWorld(),OverlayWidgetClass);
	//控件添加到视口
	Widget->AddToViewport();
	
}
