// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/LoadScreenSaveGame.h"

/**
 * @brief 根据地图名称在存档数据中查找并返回对应的地图存档信息。
 * @param InMapName 要查找的地图的资源名称。
 * @return 如果找到，返回包含该地图所有已保存 Actor 数据的 FSavedMap 结构体副本。如果找不到，返回一个默认构造的、空的 FSavedMap。
 *
 * @par 功能说明
 * 这是一个 getter 函数，它遍历 `SavedMaps` 数组，通过字符串比较来查找特定地图的存档记录。
 *
 * @par 注意事项
 * - 该函数按值返回（pass-by-value）一个 `FSavedMap` 结构体。这意味着调用者得到的是一个**副本**。对这个返回的副本进行任何修改，都**不会**影响到 `ULoadScreenSaveGame` 对象中原始的 `SavedMaps` 数组里的数据。
 * - 如果没有找到匹配的地图，函数会返回一个临时创建的空结构体。调用方需要意识到返回的可能是空数据。
 */
FSavedMap ULoadScreenSaveGame::GetSavedMapWithMapName(const FString& InMapName)
{
	// 遍历 `SavedMaps` 这个 TArray 数组中的每一个 `FSavedMap` 元素。
	// `const FSavedMap& Map` 表示我们以只读引用的方式访问每个元素，避免了不必要的拷贝，效率更高。
	for (const FSavedMap& Map : SavedMaps)
	{
		// 比较当前遍历到的地图记录的名称和我们想要查找的名称。
		if (Map.MapAssetName == InMapName)
		{
			// 如果找到了匹配项，立即返回该元素的副本。函数执行到此结束。
			return Map;
		}
	}
	// 如果遍历完整个数组都没有找到匹配的地图名，则返回一个用默认构造函数创建的、空的 FSavedMap 实例。
	return FSavedMap();
}

/**
 * @brief 检查存档数据中是否存在指定地图的记录。
 * @param InMapName 要检查的地图的资源名称。
 * @return 如果存在该地图的记录，返回 `true`；否则返回 `false`。
 *
 * @par 功能说明
 * 这是一个简单的查询函数，用于快速判断某个地图是否已经被存档过，而无需获取其完整的存档数据。
 */
bool ULoadScreenSaveGame::HasMap(const FString& InMapName)
{
	// 遍历逻辑与上一个函数完全相同。
	for (const FSavedMap& Map : SavedMaps)
	{
		// 比较地图名。
		if (Map.MapAssetName == InMapName)
		{
			// 只要找到一个匹配项，就可以确定“存在”，立即返回 true。
			return true;
		}
	}
	// 如果遍历完整个数组都没有找到任何匹配项，说明“不存在”，返回 false。
	return false;
}