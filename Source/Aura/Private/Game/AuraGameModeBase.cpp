// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/AuraGameModeBase.h"

#include "Game/AuraGameInstance.h"
#include "Game/LoadScreenSaveGame.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"

/**
 * @brief 将一个 ViewModel 中的数据保存到磁盘上的一个存档槽位。
 * @param LoadSlot 包含需要保存数据的 ViewModel 指针。
 * @param SlotIndex 存档槽位的数字索引。
 *
 * @par 功能说明
 * 该函数实现了“覆盖保存”的逻辑。它首先检查目标槽位是否已存在存档，如果存在则先删除，
 * 然后创建一个新的存档对象，从传入的 ViewModel 中复制数据，最后再将这个新对象写入磁盘。
 *
 * @par 详细流程
 * 1.  **检查并删除旧档**: 使用 `DoesSaveGameExist` 检查存档是否存在。如果存在，调用 `DeleteGameInSlot` 删除它。
 * 2.  **创建存档实例**: 调用 `CreateSaveGameObject` 在内存中创建一个新的存档对象实例。`LoadScreenSaveGameClass` 是一个蓝图中指定的 `TSubclassOf<USaveGame>`。
 * 3.  **类型转换**: 将通用的 `USaveGame` 指针安全地转换为具体的 `ULoadScreenSaveGame` 指针，以便访问其自定义的成员变量。
 * 4.  **数据拷贝**: 从 `LoadSlot` ViewModel 中获取玩家名和地图名，并赋值给 `LoadScreenSaveGame` 对象的相应变量。同时，将存档状态硬编码为 `Taken`。
 * 5.  **写入磁盘**: 调用 `SaveGameToSlot`，将填充好数据的存档对象序列化到磁盘文件中。
 */
void AAuraGameModeBase::SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex)
{
	// (为什么这么做): 这是一个“覆盖保存”的逻辑。先检查存档是否存在，如果存在就删除。
	// 这样可以确保每次保存都是一个全新的、干净的数据文件，避免了处理旧数据合并的复杂性。
	if (UGameplayStatics::DoesSaveGameExist(LoadSlot->GetLoadSlotName(), SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(LoadSlot->GetLoadSlotName(), SlotIndex);
	}
	// 从一个在蓝图中指定的类 (LoadScreenSaveGameClass) 创建一个存档对象实例。
	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	// 将基类的 USaveGame 指针转换为我们自定义的 ULoadScreenSaveGame 指针，以便访问 PlayerName 等自定义属性。
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	// 从传入的 ViewModel 中读取数据，并填充到存档对象中。
	LoadScreenSaveGame->PlayerName = LoadSlot->GetPlayerName();
	LoadScreenSaveGame->MapName = LoadSlot->GetMapName();
	LoadScreenSaveGame->SaveSlotStatus = Taken; // 将存档状态标记为“已占用”。
	LoadScreenSaveGame->PlayerStartTag = LoadSlot->PlayerStartTag;
	// 调用引擎的静态函数，将内存中的存档对象写入到磁盘。文件名由 SlotName 和 SlotIndex 共同决定。
	UGameplayStatics::SaveGameToSlot(LoadScreenSaveGame, LoadSlot->GetLoadSlotName(), SlotIndex);
}

/**
 * @brief 根据槽位名和索引从磁盘加载存档数据。
 * @param SlotName 存档槽位的名称。
 * @param SlotIndex 存档槽位的数字索引。
 * @return 指向加载或新创建的 ULoadScreenSaveGame 对象的指针。
 *
 * @par 功能说明
 * 这个函数尝试从磁盘加载一个存档文件。如果文件存在，则加载其内容；如果不存在，则在内存中创建一个全新的、空的存档对象。
 * 这样做可以确保函数总能返回一个有效的对象，简化了调用方的空指针检查。
 *
 * @par 注意事项
 * - `const` 关键字表明此函数不会修改 `AAuraGameModeBase` 类的任何成员变量。
 */
ULoadScreenSaveGame* AAuraGameModeBase::GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const
{
	USaveGame* SaveGameObject = nullptr; // 初始化为空指针。
	// 检查磁盘上是否存在对应的存档文件。
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		// 如果存在，则从磁盘加载，并将其内容反序列化到 SaveGameObject 中。
		SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, SlotIndex);
	}
	else
	{
		// 如果不存在，则在内存中创建一个新的、空的存档对象。
		SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	}
	// 无论上面哪条路径，SaveGameObject 现在都应该是一个有效的指针（除非 CreateSaveGameObject 失败）。
	// 将其转换为子类指针并返回。
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	return LoadScreenSaveGame;
}

/**
 * @brief 从磁盘删除一个指定的存档槽位文件。
 * @param SlotName 要删除的存档槽位的名称。
 * @param SlotIndex 要删除的存档槽位的索引。
 */
void AAuraGameModeBase::DeleteSlot(const FString& SlotName, int32 SlotIndex)
{
	// 在尝试删除前，先检查文件是否存在，这是一个好习惯，可以避免不必要的磁盘操作或潜在的警告。
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(SlotName, SlotIndex);
	}
}

/**
 * @brief 切换到指定存档槽位关联的地图。
 * @param Slot 包含目标地图信息的 ViewModel 指针。
 *
 * @par 注意事项
 * - `Maps.FindChecked` 会在 TMap 中查找地图名。如果找不到，程序会崩溃。这要求所有可跳转的地图必须预先注册。
 * - `OpenLevelBySoftObjectPtr` 的第一个参数 `WorldContextObject` (这里是 `Slot`) 用于获取当前的游戏世界实例。
 */
void AAuraGameModeBase::TravelToMap(UMVVM_LoadSlot* Slot)
{
	const FString SlotName = Slot->GetLoadSlotName(); // 获取存档名（虽然在此函数中未使用，但可能是为未来逻辑准备的）。
	const int32 SlotIndex = Slot->SlotIndex; // 获取槽位索引（同样未使用）。

	// 从 ViewModel 获取地图名，然后用这个名字作为 Key 在 Maps TMap 中查找对应的地图资源引用。
	// FindChecked 会在找不到 Key 时使程序崩溃，确保了地图名必须是有效的。
	// Maps 是一个 TMap<FString, TSoftObjectPtr<UWorld>>，存储着地图名 到地图资源的映射。
	// TSoftObjectPtr 是一个“软”引用，它只存储资源的路径，不会在加载时将资源一直保留在内存中。
	UGameplayStatics::OpenLevelBySoftObjectPtr(Slot, Maps.FindChecked(Slot->GetMapName()));
}

AActor* AAuraGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{

	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Actors);
	if (Actors.Num() > 0)
	{
		AActor* SelectedActor = Actors[0];
		for (AActor* Actor : Actors)
		{
			if (APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
			{
				if (PlayerStart->PlayerStartTag == AuraGameInstance->PlayerStartTag)
				{
					SelectedActor = PlayerStart;
					break;
				}
				
			}
		}
		return SelectedActor;
	}
	return nullptr;
}

/**
 * @brief 在 GameMode 初始化时被调用，用于注册默认的地图。
 */
void AAuraGameModeBase::BeginPlay()
{
	// 调用父类的 BeginPlay，确保 GameMode 的基础功能被正确初始化。
	Super::BeginPlay();
	// 将在蓝图中设置的默认地图名 (DefaultMapName) 和默认地图资源引用 (DefaultMap) 添加到 Maps TMap 中。
	// 这是为了确保至少有一个地图可供 `TravelToMap` 函数查找和跳转。
	Maps.Add(DefaultMapName, DefaultMap);
}