// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AuraHUD.generated.h"

class UAttributeSet;
class UAbilitySystemComponent;
struct FWidgetControllerParams;
class UOverlayWidgetController;
class UAuraUserWidget;
/**
 * 
 */
UCLASS()
class AURA_API AAuraHUD : public AHUD
{
	GENERATED_BODY()

public:

	//声明小部件
	UPROPERTY()
	TObjectPtr<UAuraUserWidget> OverlayWidget;

	//获取重叠控件控制器
	UOverlayWidgetController * GetOverlayWidgetController(const FWidgetControllerParams& WCParams);

	//初始化 UI 控件
	void InitOverlay(APlayerController * PC,APlayerState* APS,UAbilitySystemComponent * ASC,UAttributeSet * AS);

protected:
	

private:

	//声明覆层控件类
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraUserWidget> OverlayWidgetClass;

	//声明重叠控件控制器
	UPROPERTY()
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;

	//声明重叠控件控制器类
	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;
	
};
