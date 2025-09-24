// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraEnemySpawnPoint.h"

#include "Character/EnemyCharacter.h"



/**
 * @brief 在此生成点（Spawn Point）的位置生成一个敌人角色。
 *
 * @par 功能说明
 * 该函数负责在游戏运行时动态地创建一个敌人 Actor。它使用了“延迟生成” (Deferred Spawning)
 * 的方法，这是一种高级的 Actor 生成模式，允许我们在 Actor 的 `BeginPlay` 事件被调用之前，
 * 对其属性进行安全的初始化设置。
 *
 * @par 详细流程
 * 1.  **设置生成参数**: 创建一个 `FActorSpawnParameters` 结构体，并配置其碰撞处理方式为 `AdjustIfPossibleButAlwaysSpawn`。这意味着引擎会尝试在生成点附近寻找一个没有碰撞的位置，但如果找不到，它依然会强制生成 Actor（即使初始位置有重叠）。
 * 2.  **开始延迟生成**: 调用 `GetWorld()->SpawnActorDeferred`。这个函数会在内存中创建 `AEnemyCharacter` 的实例，但会“暂停”其初始化过程，**不会**立即调用 `BeginPlay`。
 * 3.  **初始化自定义属性**: 在这个“暂停”阶段，我们可以安全地调用新创建的 `Enemy` 实例上的自定义函数，如 `SetLevel` 和 `SetCharacterClass`，来设置它的初始等级和职业。这些值将在 `BeginPlay` 中可用。
 * 4.  **完成生成**: 调用 `Enemy->FinishSpawning`。这是延迟生成的第二部分，也是必需的步骤。它会“解禁”Actor 的初始化流程，此时 `BeginPlay` 会被调用，Actor 会被完整地添加到场景中，并开始其生命周期。
 * 5.  **生成 AI 控制器**: 调用 `Enemy->SpawnDefaultController()`。一个 Character (Pawn) 只是一个“木偶”，它需要一个 AI Controller (AI控制器) 作为“大脑”来驱动其行为（例如，运行行为树）。此函数会根据 `AEnemyCharacter` 类中设置的默认 AI Controller 类来为其生成一个控制器实例。
 *
 * @par 注意事项
 * - **延迟生成 (Deferred Spawning)** 是本函数的核心。它解决了“先有鸡还是先有蛋”的问题。如果使用常规的 `SpawnActor`，`BeginPlay` 会立即执行，此时你还没有机会设置 `EnemyLevel`，导致 `BeginPlay` 里的逻辑会使用错误的默认等级。延迟生成确保了所有前置数据都已准备就绪后，`BeginPlay` 才会被调用。
 * - `EnemyClass`, `EnemyLevel`, `CharacterClass` 都是 `AAuraEnemySpawnPoint` 上的 `UPROPERTY` 成员变量，必须在蓝图编辑器中为这个生成点的实例正确配置。
 */
void AAuraEnemySpawnPoint::SpawnEnemy()
{
	// 步骤 1/5: 准备生成参数，特别是碰撞处理方式。
	FActorSpawnParameters SpawnParameters;
	// (为什么这么做): 这是为了确保敌人总能生成成功。
	// AdjustIfPossibleButAlwaysSpawn 策略会尝试避免碰撞，但如果生成点完全被阻塞，它也会强制生成，
	// 而不是像其他策略那样直接生成失败。这对于设计好的生成点来说非常可靠。
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// 步骤 2/5: 开始延迟生成 Actor。
	// SpawnActorDeferred 会创建一个 Actor 实例，但会暂停其初始化，不会立即调用 BeginPlay。
	// EnemyClass 是在这个 SpawnPoint 蓝图中设置的、要生成的具体敌人类型。
	AEnemyCharacter* Enemy = GetWorld()->SpawnActorDeferred<AEnemyCharacter>(EnemyClass, GetActorTransform());
	
	// 步骤 3/5: 在 Actor 完全“激活”前，设置其初始属性。
	// 这是使用延迟生成的全部意义所在。这些值将在 Enemy 的 BeginPlay 中被访问。
	Enemy->SetLevel(EnemyLevel); // 设置敌人的等级。
	Enemy->SetCharacterClass(CharacterClass); // 设置敌人的职业（可能会影响其技能或属性）。
	
	// 步骤 4/5: 完成生成过程。
	// FinishSpawning 是 SpawnActorDeferred 的配对函数，必须被调用。
	// 它会触发 Actor 的 PostActorCreated 和 BeginPlay 事件，使其在游戏中完全“复活”。
	Enemy->FinishSpawning(GetActorTransform());

	// 步骤 5/5: 为新生成的敌人 AI 生成一个控制器。
	// 如果没有控制器，这个 Character 将无法执行任何 AI 逻辑（如行为树）。
	Enemy->SpawnDefaultController();
}
}
