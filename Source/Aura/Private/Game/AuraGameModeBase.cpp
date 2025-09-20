// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/AuraGameModeBase.h"

#include "EngineUtils.h"
#include "Aura/AuraLogChannels.h"
#include "Game/AuraGameInstance.h"
#include "Game/LoadScreenSaveGame.h"
#include "GameFramework/PlayerStart.h"
#include "Interation/SaveInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
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
 * @brief 从游戏实例 (GameInstance) 中获取当前激活的存档槽位信息，并加载对应的存档对象。
 * @return 指向已加载或新创建的 ULoadScreenSaveGame 对象的指针。
 *
 * @par 功能说明
 * 这是一个辅助函数，用于统一获取“当前游戏内”正在使用的存档对象。它首先从 GameInstance 读取
 * 玩家在主菜单选择的存档槽位名称和索引，然后调用另一个辅助函数 `GetSaveSlotData` 来执行
 * 实际的加载或创建操作。
 */
ULoadScreenSaveGame* AAuraGameModeBase::RetrieveInGameSaveData()
{
	// (为什么这么做): GameInstance 是一个在切换关卡时依然存在的对象。
	// 将玩家选择的存档槽信息（LoadSlotName, LoadSlotIndex）存在这里，是跨关卡传递数据的最佳实践。
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());
	const FString InGameLoadSlotName = AuraGameInstance->LoadSlotName; // 从 GameInstance 读取存档名。
	const int32 InGameLoadSlotIndex = AuraGameInstance->LoadSlotIndex; // 从 GameInstance 读取存档索引。
	
	// 调用我们之前分析过的函数，它会从磁盘加载或在内存中创建一个新的存档对象。
	return GetSaveSlotData(InGameLoadSlotName, InGameLoadSlotIndex);
}


/**
 * @brief 将一个已修改的存档对象保存到当前激活的槽位，并更新 GameInstance 中的状态。
 * @param SaveObject 包含需要保存数据的、已在内存中修改过的存档对象。
 *
 * @par 功能说明
 * 这个函数负责将高层级的游戏进度（例如玩家下一次应该出现的出生点 Tag）提交到 GameInstance，
 * 并将传入的整个 `SaveObject` 序列化到磁盘。
 */
void AAuraGameModeBase::SaveInGameProgressData(ULoadScreenSaveGame* SaveObject)
{
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());
	const FString InGameLoadSlotName = AuraGameInstance->LoadSlotName;// 同样，从 GameInstance 获取存档槽信息。
	const int32 InGameLoadSlotIndex = AuraGameInstance->LoadSlotIndex;
	// 将存档对象中的 PlayerStartTag 更新到 GameInstance 中，这可能是为了关卡切换后能立即使用。
	AuraGameInstance->PlayerStartTag = SaveObject->PlayerStartTag;
	// 将传入的 SaveObject 完整地写入磁盘。
	UGameplayStatics::SaveGameToSlot(SaveObject,InGameLoadSlotName,InGameLoadSlotIndex);
}



/**
 * @brief 保存当前世界（关卡）中所有实现了 USaveInterface 的 Actor 的状态。
 * @param World 指向当前需要保存状态的世界对象。
 *
 * @par 功能说明
 * 这是整个存档系统的核心。它会遍历当前关卡中的每一个 Actor，筛选出那些被标记为“可保存”的
 * (通过实现一个接口)，然后将这些 Actor 的位置、旋转以及所有被标记为 `SaveGame` 的属性
 * 序列化成二进制数据，最终存入存档对象中。
 * 
 * @par 详细流程
 * 1.  获取并清理当前地图的名称。
 * 2.  加载当前游戏的存档对象。
 * 3.  检查存档中是否已有本地图的记录，如果没有则创建一条新记录。
 * 4.  清空本地图之前保存的所有 Actor 数据，准备进行一次全新的“快照”保存。
 * 5.  使用 `FActorIterator` 遍历世界中的所有 Actor。
 * 6.  对每个 Actor，检查它是否实现了 `USaveInterface` 接口。
 * 7.  如果实现了，就将其 Transform (位置、旋转、缩放) 和通过 `Serialize` 函数导出的自定义数据打包到一个 `FSavedActor` 结构体中。
 * 8.  将这个结构体添加到地图的 Actor 记录中。
 * 9.  遍历结束后，将更新后的存档对象写回磁盘。
 */
void AAuraGameModeBase::SaveWorldState(UWorld* World) const
{
	// 步骤 1/5: 准备工作
	FString WorldName = World->GetMapName();// 获取关卡资源名，例如 "UEDPIE_0_L_TestMap"。
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);// 清理掉编辑器运行时的前缀，得到干净的地图名 "L_TestMap"。

	UAuraGameInstance* AuraGI = Cast<UAuraGameInstance>(GetGameInstance());
	check(AuraGI);// check() 是一个断言，如果 AuraGI 为空，程序会在开发版本中崩溃并报错。这用于强制要求 GameInstance 必须有效。

	// 使用之前的辅助函数获取当前存档对象。
	if (ULoadScreenSaveGame* SaveGame = GetSaveSlotData(AuraGI->LoadSlotName, AuraGI->LoadSlotIndex))
	{
		// 步骤 2/5: 确保地图记录存在
		if (!SaveGame->HasMap(WorldName)) // 检查存档里是否已经有这个地图的数据了。
		{
			FSavedMap NewSaveMap;// 如果没有，创建一个新的地图存档结构。
			NewSaveMap.MapAssetName = WorldName;
			SaveGame->SavedMaps.Add(NewSaveMap); // 添加到存档对象的地图列表中。
		}
		// 步骤 3/5: 遍历并序列化所有可保存的 Actor
		FSavedMap SavedMap = SaveGame->GetSavedMapWithMapName(WorldName); // 获取本地图的存档数据结构。
		SavedMap.SavedActors.Empty(); // 清空上次保存的 Actor 数据，本次保存将完全覆盖。

		for (FActorIterator It(World); It; ++It)// FActorIterator 是遍历关卡中所有 Actor 的标准工具。
		{
			AActor* Actor = *It;
			// 过滤器：Actor 必须有效，并且必须实现了 USaveInterface 接口，否则就跳过。
			if (!IsValid(Actor) || !Actor->Implements<USaveInterface>()) continue;

			FSavedActor SavedActor;// 创建一个用于存储单个 Actor 数据的结构体。
			SavedActor.ActorName = Actor->GetFName();// 保存 Actor 的唯一名称 (FName)。
			SavedActor.Transform = Actor->GetTransform();// 保存 Actor 的 Transform。


			// (为什么这么做): 这是 Unreal Engine 底层的对象序列化机制。
			// FMemoryWriter 创建一个内存写入流，指向 SavedActor.Bytes 这个字节数组。
			// FObjectAndNameAsStringProxyArchive 是一个特殊的序列化器，`ArIsSaveGame = true` 是关键，
			// 它告诉 Actor->Serialize()：“请只把标记了 UPROPERTY(SaveGame) 的变量写入到流中”。
			FMemoryWriter MemoryWriter(SavedActor.Bytes);
			FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
			Archive.ArIsSaveGame = true;
			Actor->Serialize(Archive);// Actor 将自己的 SaveGame 属性写入到 SavedActor.Bytes 中。

			// 将打包好的 Actor 数据添加到地图存档中。
			SavedMap.SavedActors.AddUnique(SavedActor);
			
		}
		// 步骤 4/5: 将修改后的地图数据写回存档对象
		// (为什么这么做): `GetSavedMapWithMapName` 可能返回的是一个结构体的副本 (copy)。
		// 所以对 `SavedMap` 的所有修改都只是在副本上，需要把这个副本写回原始的 `SaveGame->SavedMaps` 数组中。
		for (FSavedMap& MapToReplace : SaveGame->SavedMaps)
		{
			if (MapToReplace.MapAssetName == WorldName)
			{
				MapToReplace = SavedMap;
			}
		}
		// 步骤 5/5: 将包含最新世界状态的存档对象写入磁盘。
		UGameplayStatics::SaveGameToSlot(SaveGame,AuraGI->LoadSlotName, AuraGI->LoadSlotIndex);
	}
}

/**
 * @brief 加载并应用当前世界中所有 Actor 的已保存状态。
 * @param World 指向当前需要加载状态的世界对象。
 *
 * @par 功能说明
 * 这是 `SaveWorldState` 的逆向操作。它会加载存档文件，遍历当前关卡中所有可被加载的 Actor，
 * 然后在存档数据中查找与之一一对应的记录。找到后，它会将保存的 Transform 和自定义数据
 * 反序列化回 Actor，从而恢复其之前的状态。
 */
void AAuraGameModeBase::LoadWorldState(UWorld* World) const
{
	FString WorldName = World->GetMapName();// 同样，获取并清理地图名。
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);
	
	UAuraGameInstance* AuraGI = Cast<UAuraGameInstance>(GetGameInstance());
	check(AuraGI);


	if (UGameplayStatics::DoesSaveGameExist(AuraGI->LoadSlotName, AuraGI->LoadSlotIndex))
	{
		ULoadScreenSaveGame* SaveGame = Cast<ULoadScreenSaveGame>(UGameplayStatics::LoadGameFromSlot(AuraGI->LoadSlotName, AuraGI->LoadSlotIndex));
		if (SaveGame == nullptr)
		{
			UE_LOG(LogAura, Error, TEXT("加载存档失败"));
			return;
		}
		// 步骤 1/3: 遍历世界中的 Actor
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor->Implements<USaveInterface>()) continue;

			// 步骤 2/3: 在存档数据中查找匹配的 Actor
			// 这里是一个嵌套循环，遍历本地图存档中的每一个 Actor 记录。
			for (FSavedActor SavedActor : SaveGame->GetSavedMapWithMapName(WorldName).SavedActors)
			{
				// 通过 Actor 的唯一名称 FName 进行匹配。
				if (SavedActor.ActorName == Actor->GetFName())
				{
					// 步骤 3/3: 应用数据
					// (为什么这么做): 通过接口调用 Actor 自身的函数，让 Actor 自己决定是否要加载 Transform。
					// 例如，一个敌人可能在被杀死后保存了位置，但我们希望它在加载时重新从刷新点出现，这时就应该返回 false。
					if (ISaveInterface::Execute_ShouldLoadTransform(Actor))
					{
						Actor->SetActorTransform(SavedActor.Transform);
					}
					// 这是反序列化过程，与保存时完全对应。
					// FMemoryReader 从保存的字节数组中读取数据。
					// Archive 被配置为加载模式。
					// Actor->Serialize(Archive) 会从流中读取数据，并填充到自己的 SaveGame 属性中。
					FMemoryReader MemoryReader(SavedActor.Bytes);
					FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
					Archive.ArIsSaveGame = true;
					Actor->Serialize(Archive);

					// (为什么这么做): 这是一个“加载后”的回调。在所有数据都恢复后，调用接口的 LoadActor 函数。
					// Actor 可以在这个函数里执行一些额外的逻辑，比如根据新加载的 bIsOpened 状态来更新自己的模型或材质。
					ISaveInterface::Execute_LoadActor(Actor);
				}
			}
		}
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


/**
 * @brief 覆盖默认的玩家出生点选择逻辑，根据 GameInstance 中存储的 PlayerStartTag 来选择一个特定的 APlayerStart。
 * @param Player 正在进入游戏的玩家的控制器 (Controller)。
 * @return 指向被选中的 APlayerStart Actor 的指针。如果找不到匹配的，则返回找到的第一个 APlayerStart 或 nullptr。
 *
 * @par 功能说明
 * 这是 `AGameModeBase` 中的一个虚函数 `ChoosePlayerStart` 的实现 (`_Implementation`)。
 * 默认情况下，引擎会随机选择一个场景中的 `APlayerStart` Actor 作为出生点。
 * 这段代码重写了该逻辑，使其能够根据一个 `FName` 标签（Tag）来精确地选择出生点。
 * 这个标签通常是在关卡切换前被设置到 `GameInstance` 中的，例如，当玩家从“村庄”进入“地下城”时，
 * `PlayerStartTag` 就会被设置为“FromVillage_Entrance”，以确保玩家出现在地下城中正确的入口位置。
 *
 * @par 详细流程
 * 1.  **获取 GameInstance**: 从中读取 `PlayerStartTag`，这是我们选择出生点的依据。
 * 2.  **查找所有出生点**: 使用 `UGameplayStatics::GetAllActorsOfClass` 收集当前关卡中所有的 `APlayerStart` Actor。
 * 3.  **设置默认选项**: 如果找到了至少一个 `APlayerStart`，先将第一个作为默认的备选出生点。
 * 4.  **遍历并匹配**: 循环遍历所有找到的 `APlayerStart` Actor。
 * 5.  **比较 Tag**: 对每一个 `APlayerStart`，获取其自身的 `PlayerStartTag` 属性，并与 `GameInstance` 中存储的 Tag 进行比较。
 * 6.  **选择并中断**: 如果找到了一个 Tag 完全匹配的 `APlayerStart`，就将其选为最终的出生点，并立即用 `break` 退出循环。
 * 7.  **返回结果**: 返回最终被选中的 `APlayerStart`。如果遍历完都没有找到匹配的 Tag，则返回之前设置的默认出生点。如果场景中一个 `APlayerStart` 都没有，则返回 `nullptr`。
 */
AActor* AAuraGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{

	// 步骤 1/4: 从 GameInstance 获取目标出生点的 Tag。
	// GameInstance 是一个跨关卡持续存在的对象，非常适合存储这种临时状态。
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());
	// 步骤 2/4: 获取关卡中所有的 APlayerStart Actor
	TArray<AActor*> Actors;
	// 这是一个引擎提供的静态函数，用于根据类来查找场景中的所有 Actor。
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Actors);
	// 步骤 3/4: 如果找到了任何出生点，就开始进行筛选
	if (Actors.Num() > 0)
	{
		// (为什么这么做): 这是一个很好的防御性措施。
		// 先将找到的第一个 APlayerStart 设为默认选中项。这样，即使后面没有找到任何 Tag 匹配的出生点，
		// 游戏也不会因为找不到出生点而崩溃，玩家至少会有一个地方出生。
		AActor* SelectedActor = Actors[0];

		// 遍历所有找到的 APlayerStart Actor
		for (AActor* Actor : Actors)
		{
			// 将通用的 AActor* 安全地转换为 APlayerStart*，以便访问其特有的 PlayerStartTag 属性。
			if (APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
			{
				// 核心逻辑：比较 PlayerStart Actor 自身的 Tag 和我们从 GameInstance 中取出的目标 Tag。
				if (PlayerStart->PlayerStartTag == AuraGameInstance->PlayerStartTag)
				{
					SelectedActor = PlayerStart;// 如果匹配，就更新选中项。
					break;// (为什么这么做): 既然已经找到了我们想要的那个唯一的出生点，就没必要再继续遍历剩下的了。break 可以立即跳出循环，提升效率。
				}
				
			}
		}
		// 步骤 4/4: 返回最终选中的 Actor。
		return SelectedActor;
	}
	// 如果场景中连一个 APlayerStart 都没有，就返回空指针。这可能会导致生成玩家失败。
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