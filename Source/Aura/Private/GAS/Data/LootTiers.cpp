


#include "GAS/Data/LootTiers.h"



/**
 * @brief 根据预设的掉落表和概率计算并返回本次生成的战利品列表。
 * @return 一个包含所有本次成功生成的战利品信息的 TArray<FLootItem>。
 *
 * @par 功能说明
 * 该函数属于一个“掉落等级 (Loot Tiers)”数据资产。这个数据资产中包含了一个 `LootItems` 数组，
 * 定义了所有可能掉落的物品及其相关概率。当外部系统（例如一个被击杀的敌人或一个被打开的宝箱）
 * 需要生成战利品时，就会调用这个函数。
 *
 * @par 详细流程
 * 1.  **创建返回数组**: 初始化一个空的 `ReturnItems` 数组，用于存储最终生成的物品。
 * 2.  **遍历掉落表**: 遍历 `LootItems` 数组中的每一种可能的掉落物 (`Item`)。
 * 3.  **多次判定**: 对于每一种掉落物，根据其 `MaxNumberToSpawn`（最大生成数量）进行多次独立的掉落判定。例如，一个物品最大可能掉落3个，就会进行3次判定。
 * 4.  **概率检定**: 在每次判定中，生成一个 1 到 100 之间的随机浮点数。
 * 5.  **比较概率**: 将这个随机数与物品预设的 `ChanceToSpawn`（生成概率）进行比较。
 * 6.  **成功生成**: 如果随机数小于生成概率，则认为本次掉落成功：
 *     - 创建一个新的 `FLootItem` 实例。
 *     - 将原物品的类别 (`LootClass`) 和等级覆盖标志 (`bLootLevelOverride`) 复制到新实例中。
 *     - 将这个新实例添加到 `ReturnItems` 数组中。
 * 7.  **返回结果**: 在遍历完所有物品和所有判定次数后，返回包含了所有成功生成的战利品的 `ReturnItems` 数组。
 */
TArray<FLootItem> ULootTiers::  GetLootItems()
{
	// 创建一个 TArray 用于存储最终要返回的物品列表。
	TArray<FLootItem> ReturnItems;

	// 遍历此掉落表 (LootTiers) 中定义的所有可能的掉落项。
	// `LootItems` 是一个 UPROPERTY 的 TArray<FLootItem>，在数据资产编辑器中配置。
	// (注意): `for (FLootItem& Item ...)` 使用了引用，避免了不必要的拷贝，是好的实践。
	for (FLootItem& Item : LootItems)
	{
		// 对于每一种物品，根据其“最大生成数量”进行多次尝试。
		// 例如，如果 MaxNumberToSpawn 是 3，这个内层循环就会执行 3 次。
		for (int32 i = 0; i < Item.MaxNumberToSpawn; ++i)
		{
			// FMath::FRandRange(1.f, 100.f) 生成一个 1.0 到 100.0 之间的随机浮点数。
			// Item.ChanceToSpawn 是在编辑器中设置的掉落概率 (例如 25.0 代表 25%)。
			// 如果随机数小于概率，则判定成功。
			if (FMath::FRandRange(1.f, 100.f) < Item.ChanceToSpawn)
			{
				// --- 掉落成功 ---
				FLootItem NewItem;// 创建一个新的物品实例来添加到返回列表中。
				// (为什么这么做): 我们不直接添加 `Item`，因为 `Item` 中可能包含 `MaxNumberToSpawn` 等
				// 判定用的数据。返回的列表中每个元素应该只代表一个独立的掉落物。
				NewItem.LootClass = Item.LootClass; // 复制物品的蓝图类。
				NewItem.bLootLevelOverride = Item.bLootLevelOverride;// 复制等级覆盖标志。
				ReturnItems.Add(NewItem); // 将新创建的物品实例添加到返回数组中。
			}
		}
	}
	// 返回最终生成的物品列表。如果运气不好，这个数组可能是空的。
	return ReturnItems;
}
