// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/SaveGame.h"
#include "LoadScreenSaveGame.generated.h"

class UGameplayAbility;

UENUM(BlueprintType) // 将这个 C++ 枚举暴露给蓝图，使其可以在蓝图中作为变量类型使用
// 定义存档槽的状态
enum ESaveSlotStatus
{
	Vacant,     // 状态：空的
	EnterName,  // 状态：等待输入名称 (此状态在之前的代码中未使用，可能是为未来功能预留)
	Taken       // 状态：已被占用
};

USTRUCT(BlueprintType)
// 定义了用于保存单个技能所有相关数据的结构体
struct FSavedAbility
{
	GENERATED_BODY() // UE 结构体必需的宏，用于支持反射系统

	// 技能的 C++ 类。TSubclassOf 是一种安全的指针，确保只能指定 UGameplayAbility 或其子类
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClassDefaults")
	TSubclassOf<UGameplayAbility> GamePlayAbility;

	// 技能的唯一标识 Tag
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityTag = FGameplayTag();

	// 技能的当前状态 Tag (例如：已装备, 未装备, 冷却中)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityStatus = FGameplayTag();

	// 技能被装备到的槽位 Tag (例如：快捷键1, 鼠标左键)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilitySlot = FGameplayTag();

	// 技能的类型 Tag (例如：主动, 被动)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityType = FGameplayTag();

	// 技能的当前等级
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 AbilityLevel;
};


/**
 * @brief 重载 "等于" (==) 操作符，用于比较两个 FSavedAbility 结构体。
 * @param Left 左操作数 (FSavedAbility 实例)。
 * @param Right 右操作数 (FSavedAbility 实例)。
 * @return 如果两个结构体被视为相等，则返回 true；否则返回 false。
 *
 * @par 功能说明
 * 这段代码为自定义的 `FSavedAbility` 结构体提供了一个自定义的相等性比较逻辑。
 * 它并没有逐一比较结构体的所有成员（如 AbilityLevel, AbilitySlot 等），
 * 而是**只比较**它们的 `AbilityTag` 成员。这意味着，在所有使用 `==` 操作符的上下文中
 * （例如在 TArray::Find, TArray::Contains, TArray::Remove 等算法中），
 * 两个 `FSavedAbility` 实例只要它们的 `AbilityTag` 完全相同，就会被认为是“相等”的，
 * 即使它们的等级或其他属性不同。
 *
 * @par 详细流程
 * 1.  接收左边和右边两个 `FSavedAbility` 实例的常量引用。
 * 2.  调用 `FGameplayTag::MatchesTagExact()` 函数来精确比较 `Left.AbilityTag` 和 `Right.AbilityTag`。
 * 3.  返回比较的结果（`true` 或 `false`）。
 *
 * @par 注意事项
 * - `inline` 关键字建议编译器将这个函数的代码直接插入到调用它的地方，对于这样短小的函数，这可以轻微地提升性能，避免函数调用的开销。
 * - 这种“代理比较”（只比较一个核心成员）的设计非常常见，但也需要特别小心。开发者必须清楚地知道，“相等”的定义已经被简化了。
 */

inline bool operator==(const FSavedAbility& Left, const FSavedAbility& Right)
{
	
	// (为什么这么做): 这里定义了两个 FSavedAbility 结构体 "相等" 的标准。
	// 它没有比较结构体里的每一个字段（如等级、类型等），而是只关心核心的 AbilityTag。
	// 这样做是为了在查找、删除数组元素等操作中，可以只根据技能的唯一标识 Tag 来进行，而忽略其动态变化的等级等属性。
	// MatchesTagExact 是 FGameplayTag 提供的函数，用于进行精确、高效的 Tag 比较。
	return Left.AbilityTag.MatchesTagExact(Right.AbilityTag);
}

USTRUCT()
// 定义了用于保存单个 Actor 状态的结构体
struct FSavedActor
{
	GENERATED_BODY()

	// Actor 在关卡中的唯一名称。这是在加载时匹配 Actor 的关键。
	UPROPERTY()
	FName ActorName = FName();

	// Actor 的世界变换信息 (位置, 旋转, 缩放)。
	UPROPERTY()
	FTransform Transform = FTransform();

	// 用于存储 Actor 自定义属性的二进制数据流。
	// 所有被标记为 UPROPERTY(SaveGame) 的变量都会被序列化到这个字节数组中。
	UPROPERTY()
	TArray<uint8> Bytes;
};


// 重载“等于”(==)操作符，用于比较两个 FSavedActor 实例
inline bool operator==(const FSavedActor& Left, const FSavedActor& Right)
{
	// 两个 Actor 存档数据是否相等，仅由它们的 ActorName 决定。
	return  Left.ActorName == Right.ActorName ;
}

USTRUCT()
// 定义了用于保存单个地图（关卡）所有 Actor 状态的结构体
struct FSavedMap
{
	GENERATED_BODY()

	// 地图资源的名称，例如 "L_MainVillage"。
	UPROPERTY()
	FString MapAssetName = FString();

	// 一个数组，包含了这个地图中所有被保存的 Actor 的数据。
	UPROPERTY()
	TArray<FSavedActor> SavedActors;
};
/**
 * 这是游戏的主要存档对象类，继承自 USaveGame。
 * 它包含了需要持久化保存的所有游戏进度数据。
 */
UCLASS()
class AURA_API ULoadScreenSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// -- 存档元数据 --
	//插槽名称
	UPROPERTY()
	FString SlotName = FString();// 存档槽的内部名称, 例如 "LoadSlot_0"

	//插槽索引
	UPROPERTY()
	int32 SlotIndex = 0;// 存档槽的数字索引

	//玩家名称
	UPROPERTY()
	FString PlayerName = FString("Default Name"); // 玩家在UI上输入或显示的名字

	//地图名称
	UPROPERTY()
	FString MapName = FString("Default Map Name"); // 玩家最后所在的地图名 (用于UI显示)

	UPROPERTY()
	FName PlayerStartTag;// 玩家下次加载时应该出现的出生点 Tag
	
	//存档槽状态
	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = Vacant; // 当前存档槽的状态 (空/已占用)

	//第一次加载数据
	UPROPERTY()
	bool bFirstTimeLoadIn = true;// 是否是第一次玩这个存档 (用于区分新游戏和加载游戏)

	/*
	 * 玩家核心进度 (通常与 PlayerState 对应)
	 */
	UPROPERTY()
	int32 PlayerLevel = 1;// 玩家等级

	UPROPERTY()
	int32 XP = 0;// 玩家经验值

	UPROPERTY()
	int32 SpellPoints = 0;// 技能点数

	UPROPERTY()
	int32 AttributePoints = 0;// 属性点数

	/*
	 * 核心属性 (通常与 AttributeSet 对应)
	 */
	UPROPERTY()
	float Strength = 0;// 力量

	UPROPERTY()
	float Intelligence = 0;// 智力

	UPROPERTY()
	float Resilience = 0;// 韧性

	UPROPERTY()
	float Vigor = 0;// 活力

	/*
	 * 技能和世界状态
	 */
	UPROPERTY()
	TArray<FSavedAbility> SavedAbilities;// 玩家当前拥有的所有技能的列表

	UPROPERTY()
	TArray<FSavedMap> SavedMaps; // 所有已探索并保存过的地图的状态列表

	// 根据地图名查找并返回地图的存档数据
	FSavedMap GetSavedMapWithMapName(const FString& InMapName);
	// 检查是否存在指定地图的存档数据
	bool HasMap(const FString& InMapName);
};
