// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AuraGameModeBase.generated.h"

class ULoadScreenSaveGame;
class USaveGame;
class UMVVM_LoadSlot;
class UCharacterClassInfo;
class UAbilityInfo;
/**
 * 
 */
UCLASS()
class AURA_API AAuraGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category="Character Class Defaults")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;
	
	UPROPERTY(EditDefaultsOnly, Category="=Ability Info")
	TObjectPtr<UAbilityInfo> AbilityInfo;

	//保存存档数据
	void SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex);

	//获取保存游戏数据
	ULoadScreenSaveGame* GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const;

	//删除保存数据
	static void DeleteSlot(const FString& SlotName, int32 SlotIndex);

	//检索游戏保存数据
	ULoadScreenSaveGame* RetrieveInGameSaveData();

	//保存在游戏进度数据中
	void SaveInGameProgressData(ULoadScreenSaveGame* SaveObject);

	//保存世界状态
	void SaveWorldState(UWorld* World, const FString& DestinationMapAssetName = FString("")) const;
	//加载世界状态
	void LoadWorldState(UWorld* World) const;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<USaveGame> LoadScreenSaveGameClass;

	UPROPERTY(EditDefaultsOnly)
	FString DefaultMapName;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> DefaultMap;

	UPROPERTY(EditDefaultsOnly)
	TMap<FString, TSoftObjectPtr<UWorld>> Maps;

	UPROPERTY(EditDefaultsOnly)
	FName DefaultPlayerStartTag;

	//打开地图
	void TravelToMap(UMVVM_LoadSlot* Slot);

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	FString GetMapNameFromMapAssetName(const FString& MapAssetName) const;

protected:
	virtual void BeginPlay() override;
};
