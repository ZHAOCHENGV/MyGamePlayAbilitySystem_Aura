// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/MVVM_LoadScreen.h"

#include "Game/AuraGameInstance.h"
#include "Game/AuraGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"




/**
 * @brief 初始化所有存档槽位的 ViewModel，并设置总槽位数量。
 *
 * @par 功能说明
 * 该函数负责创建加载界面所需的、代表每个存档槽位的 ViewModel 对象。
 * 它目前是硬编码（Hard-coded）创建三个槽位，为每个槽位设置唯一的名称和索引，
 * 然后将它们存储在一个 TMap 中以供快速查找。最后，它会更新并通知 UI 总槽位数量。
 *
 * @par 注意事项
 * - 此函数的硬编码实现使得增减存档槽位需要修改 C++ 代码，缺乏灵活性。
 * - `LoadSlot_0`, `_1`, `_2` 应该是该类的成员变量，用于直接访问。
 */
void UMVVM_LoadScreen::InitializeLoadSlots()
{
	// 步骤 1/4: 创建第一个存档槽 (Slot 0)
	LoadSlot_0 = NewObject<UMVVM_LoadSlot>(this,LoadSlotViewModelClass);// 使用 NewObject 动态创建一个 UMVVM_LoadSlot 实例，Outer 为 this。
	LoadSlot_0->SetLoadSlotName("LoadSlot_0");// 设置用于存档文件名的内部名称。
	LoadSlot_0->SlotIndex = 0, // 设置该槽位的数字索引。
	LoadSlots.Add(0, LoadSlot_0);// 将 ViewModel 实例添加到 TMap 中，键为 0。

	// 步骤 2/4: 创建第二个存档槽 (Slot 1)
	LoadSlot_1 = NewObject<UMVVM_LoadSlot>(this,LoadSlotViewModelClass);
	LoadSlot_1->SetLoadSlotName("LoadSlot_1");
	LoadSlot_1->SlotIndex = 1,
	LoadSlots.Add(1, LoadSlot_1);

	// 步骤 3/4: 创建第三个存档槽 (Slot 2)
	LoadSlot_2 = NewObject<UMVVM_LoadSlot>(this,LoadSlotViewModelClass);
	LoadSlot_2->SetLoadSlotName("LoadSlot_2");
	LoadSlot_2->SlotIndex = 2,
	LoadSlots.Add(2, LoadSlot_2);

	// 步骤 4/4: 更新并通知总槽位数量
	// (为什么这么做): 调用 MVVM 的 Setter 函数。这不仅会更新 NumLoadSlots 变量，
	// 还会通过 FieldNotify 系统通知任何绑定到这个变量的 UI 控件进行更新。
	SetNumLoadSlots(LoadSlots.Num());
}


/**
 * @brief 根据索引号安全地获取一个存档槽位的 ViewModel。
 * @param Index 要获取的槽位的索引。
 * @return 指向对应 UMVVM_LoadSlot 实例的指针。如果找不到，程序会崩溃。
 *
 * @par 注意事项
 * - `const` 关键字表示此函数是只读的，不会修改任何类成员。
 * - `FindChecked` 假定调用者总能提供有效的索引。这是一个“快速失败”的设计，
 *   有助于在开发阶段立即捕获到逻辑错误，但在发布版本中可能不够健壮。
 */
UMVVM_LoadSlot* UMVVM_LoadScreen::GetLoadSlotViewModelByIndex(int32 Index) const
{
	// FindChecked 会在 TMap 中查找 Key。如果找不到，程序会断言失败并立即崩溃。
	return LoadSlots.FindChecked(Index);
}


/**
 * @brief 处理玩家在某个空槽位上点击“创建新存档”并输入名字后的逻辑。
 * @param Slot 发生操作的槽位索引。
 * @param EnteredName 玩家输入的新存档名称。
 *
 * @par 详细流程
 * 1. 获取当前游戏的 GameMode 实例，并安全地转换为自定义的 AuraGameModeBase 类型。
 * 2. 如果转换成功，则更新对应槽位 ViewModel 的地图名和玩家名。
 * 3. 调用 GameMode 的函数，将这个 ViewModel 的数据保存到磁盘。
 * 4. 更新槽位 ViewModel 的本地状态为“已占用 (Taken)”。
 * 5. 调用槽位 ViewModel 的 `InitializeSlot` 函数，这很可能会触发一次 UI 刷新。
 */
void UMVVM_LoadScreen::NewSlotButtonPressed(int32 Slot, const FString& EnteredName)
{
	// 步骤 1/2: 获取 GameMode 并检查其有效性
	// (为什么这么做): 游戏的核心逻辑（如存盘、读盘）通常放在 GameMode 中，因为它在服务器上是权威的。
	// Cast<> 是一种安全的类型转换，如果 UGameplayStatics::GetGameMode(this) 返回的不是 AAuraGameModeBase 或其子类，AuraGameMode 会是 nullptr。
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (AuraGameMode)// 这是一个关键的安全检查，防止 GameMode 类型不匹配时程序崩溃。
	{
		// 步骤 2/2: 更新 ViewModel 并请求保存
		LoadSlots[Slot]->SetMapName(AuraGameMode->DefaultMapName);// 从 GameMode 获取默认地图名并设置到 ViewModel。
		LoadSlots[Slot]->SetPlayerName(EnteredName);// 将玩家输入的名字设置到 ViewModel。
		LoadSlots[Slot]->SetPlayerLevel(1);
		LoadSlots[Slot]->SlotStatus = Taken;// 更新 ViewModel 的内部状态为“已占用”。
		LoadSlots[Slot]->PlayerStartTag = AuraGameMode->DefaultPlayerStartTag;
		
		AuraGameMode->SaveSlotData(LoadSlots[Slot], Slot);// 调用 GameMode 的功能，传入 ViewModel 和槽位索引来执行实际的存档操作。
		LoadSlots[Slot]->InitializeSlot();// 通知该槽位根据新状态刷新其 UI。
		
		UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(AuraGameMode->GetGameInstance());
		AuraGameInstance->PlayerStartTag = AuraGameMode->DefaultPlayerStartTag;
		AuraGameInstance->LoadSlotName = LoadSlots[Slot]->GetLoadSlotName();
		AuraGameInstance->LoadSlotIndex = LoadSlots[Slot]->SlotIndex;
	}
}


/**
 * @brief 处理当玩家点击某个槽位上的“开始新游戏”按钮时的逻辑。
 *
 * @param Slot 发生操作的槽位索引。
 * @par 功能说明
 * 这个函数通过广播一个委托来通知 UI 层进行状态切换。这是一种典型的 MVVM 通信方式，
 * ViewModel 只负责发出“信号”，而不管 UI 具体如何响应，实现了良好的解耦。
 */
void UMVVM_LoadScreen::NewGameButtonPressed(int32 Slot)
{
	// LoadSlots[Slot] 访问 TMap 中指定索引的槽位 ViewModel。
	// .SetWidgetSwitcherIndex 是一个委托（Delegate）。
	// .Broadcast(1) 执行所有绑定到该委托的函数，并将整数 1 作为参数传递过去。
	// 在 UMG 中，这通常用于切换 Widget Switcher 的 Active Widget Index，例如从“空槽位”界面切换到“输入名字”界面。
	LoadSlots[Slot]->SetWidgetSwitcherIndex.Broadcast(1);
}


/**
 * @brief 处理当玩家点击一个已有存档的槽位时的选择逻辑。
 * @param Slot 被选择的槽位索引。
 *
 * @par 详细流程
 * 1. 广播一个全局的 `SlotSelected` 委托，通知可能有其他监听者（例如主“Play”按钮）发生了选择事件。
 * 2. 遍历所有的存档槽位。
 * 3. 将被点击的槽位的“选择”按钮禁用，以提供视觉反馈。
 * 4. 将所有其他槽位的“选择”按钮启用，确保玩家可以切换选择。
 * 5. 将当前选中的槽位 ViewModel 缓存在 `SelectedSlot` 成员变量中，供后续操作（如“Play”或“Delete”）使用。
 */
void UMVVM_LoadScreen::SelectSlotButtonPressed(int32 Slot)
{
	SlotSelected.Broadcast();// 广播“槽位已被选择”事件。

	// 遍历 LoadSlots TMap 中的每一个键值对（TTuple）。
	for (const TTuple<int32, UMVVM_LoadSlot*> LoadSlot : LoadSlots)
	{
		// 检查当前遍历到的槽位是否是刚刚被点击的那个。
		if (LoadSlot.Key == Slot)
		{
			LoadSlot.Value->EnableSelectSlotButton.Broadcast(false);// 如果是，则广播委托禁用其按钮。
		}
		else
		{
			LoadSlot.Value->EnableSelectSlotButton.Broadcast(true);// 如果不是，则广播委托启用其按钮。
		}
	}
	SelectedSlot = LoadSlots[Slot];// 将被选中的槽位 ViewModel 存储起来供后续使用。
}

/**
 * @brief 从磁盘加载所有存档槽位的数据，并更新对应的 ViewModel。
 *
 * @par 详细流程
 * 1. 获取 GameMode 实例。
 * 2. 遍历所有已初始化的槽位 ViewModel。
 * 3. 对每个槽位，调用 GameMode 的 `GetSaveSlotData` 函数，传入槽位名和索引来尝试从磁盘加载数据。
 * 4. 从返回的存档对象（`ULoadScreenSaveGame`）中提取数据（如状态、玩家名、地图名）。
 * 5. 将这些加载到的数据设置回对应的槽位 ViewModel 中。
 * 6. 调用 `InitializeSlot` 来根据加载的数据刷新 UI。
 */
void UMVVM_LoadScreen::LoadData()
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
    // 在循环外获取一次 GameMode，避免在循环内重复获取，效率更高。
	if (!AuraGameMode) return; // 如果获取失败，直接中断函数。

	for (const TTuple<int32, UMVVM_LoadSlot*> LoadSlot : LoadSlots)
	{
		// 调用 GameMode 的功能，尝试加载与当前槽位匹配的存档文件。
		ULoadScreenSaveGame* SaveObject = AuraGameMode->GetSaveSlotData(LoadSlot.Value->GetLoadSlotName(), LoadSlot.Key);

		// 从加载的存档数据中恢复状态
		const TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = SaveObject->SaveSlotStatus;
		LoadSlot.Value->SlotStatus = SaveSlotStatus;
		LoadSlot.Value->SetPlayerName(SaveObject->PlayerName);
		LoadSlot.Value->SetMapName(SaveObject->MapName);
		LoadSlot.Value->PlayerStartTag = SaveObject->PlayerStartTag;
		LoadSlot.Value->SetPlayerLevel(SaveObject->PlayerLevel);
		// 根据加载的数据刷新槽位 UI
		LoadSlot.Value->InitializeSlot();
	}
}

/**
 * @brief 处理删除按钮的点击事件，删除当前选中的存档。
 *
 * @par 详细流程
 * 1. 使用 `IsValid` 检查是否存在一个当前被选中的槽位 (`SelectedSlot`)。
 * 2. 如果存在，调用 GameMode 的静态函数 `DeleteSlot` 来从磁盘删除对应的存档文件。
 * 3. 将 `SelectedSlot` 的 ViewModel 状态更新为“空的 (Vacant)”。
 * 4. 调用 `InitializeSlot` 来刷新该槽位的 UI，使其显示为空槽位状态。
 * 5. 重新启用该槽位的选择按钮。
 */
void UMVVM_LoadScreen::DelectButtonPressed()
{
	// IsValid 是一个安全检查，确保 SelectedSlot 指针不是 nullptr 且其指向的对象未被标记为销毁。
	if (IsValid(SelectedSlot))
	{
		// 调用一个静态函数来执行删除操作。静态函数可以直接通过类名调用，无需类的实例。
		AAuraGameModeBase::DeleteSlot(SelectedSlot->GetLoadSlotName(),SelectedSlot->SlotIndex);
		SelectedSlot->SlotStatus = Vacant;// 更新 ViewModel 状态。
		SelectedSlot->InitializeSlot();// 刷新 UI。
		SelectedSlot->EnableSelectSlotButton.Broadcast(true);// 使其可以被再次选择。
	}
}

/**
 * @brief 处理“开始游戏”按钮的点击事件，加载选中的存档并进入游戏。
 */
void UMVVM_LoadScreen::PlayButtonPressed()
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(AuraGameMode->GetGameInstance());
	AuraGameInstance->PlayerStartTag = SelectedSlot->PlayerStartTag;
	AuraGameInstance->LoadSlotName = SelectedSlot->GetLoadSlotName();
	AuraGameInstance->LoadSlotIndex = SelectedSlot->SlotIndex;
	// 再次检查 SelectedSlot 是否有效，这是一个很好的防御性编程习惯。
	if (IsValid(SelectedSlot) && AuraGameMode)
	{
		// 请求 GameMode 执行地图跳转，并传入选中的槽位 ViewModel 作为参数，
		// GameMode 内部会从这个 ViewModel 中读取需要加载的地图名等信息。
		AuraGameMode->TravelToMap(SelectedSlot);
	}
}

/**
 * @brief MVVM Setter 函数，用于设置总槽位数量并通知 UI。
 */
void UMVVM_LoadScreen::SetNumLoadSlots(int32 InNumLoadSlots)
{
	// 使用 MVVM 核心宏。它会检查新值与旧值是否不同，如果不同，则赋值并广播 FieldNotify 通知。
	UE_MVVM_SET_PROPERTY_VALUE(NumLoadSlots, InNumLoadSlots);
}