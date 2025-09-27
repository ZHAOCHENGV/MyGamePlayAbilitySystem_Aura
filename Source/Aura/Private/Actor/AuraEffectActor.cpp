// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetMathLibrary.h"


AAuraEffectActor::AAuraEffectActor()
{ 
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}

void AAuraEffectActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// 步骤 1/3: 累加运行时间
	RunningTime += DeltaTime;// RunningTime 用于正弦曲线运动的计算。
	// 步骤 2/3: 重置运行时间 
	const float SinePeriod = 2 * PI / SinePeriodConstant;
	// 当前逻辑是用 SinePeriodConstant 来重置时间。
	if (RunningTime > SinePeriodConstant)
	{
		RunningTime = 0.f;
	}
	// 步骤 3/3: 调用移动计算函数
	ItemMovement(DeltaTime);
}


void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
	// 缓存 Actor 在关卡中被放置的初始位置和旋转。
	// 这是正弦曲线运动的基准点。
	InitialLocation = GetActorLocation();
	CalculatedLocation = InitialLocation;// 初始化计算后的位置为初始位置。
	CalculatedRotation = GetActorRotation();// 初始化计算后的旋转为初始旋转。
}

/**
 * @brief 外部调用的函数，用于启动上下浮动效果。
 */
void AAuraEffectActor::StartSinusoidalMovement()
{
	// 开启正弦移动的标志位。
	bSinusoidalMovement = true;
	// 重新缓存调用此函数时的位置作为新的运动基准点。
	InitialLocation = GetActorLocation();
	CalculatedLocation = InitialLocation; 
}

/**
 * @brief 外部调用的函数，用于启动旋转效果。
 */
void AAuraEffectActor::StartRotation()
{
	bRotates = true;// 开启旋转的标志位。
	// 重新缓存调用此函数时的旋转作为新的旋转基准。
	CalculatedRotation = GetActorRotation();
}

/**
 * @brief 核心计算函数，根据当前状态计算出下一帧的位置和旋转。
 * @param DeltaTime 帧间隔时间。
 */
void AAuraEffectActor::ItemMovement(float DeltaTime)
{
	// 如果 bRotates 为 true，则计算旋转。
	if (bRotates)
	{
		// 计算这一帧应该旋转的角度。RotationRate 是一个 float，表示每秒旋转多少度。
		const FRotator DeltaRotation (0.f, DeltaTime * RotationRate, 0.f);
		// UKismetMathLibrary::ComposeRotators 用于将两个旋转组合在一起。
		// 这里是在上一帧的旋转基础上，再增加 DeltaRotation 的旋转量。
		CalculatedRotation = UKismetMathLibrary::ComposeRotators(CalculatedRotation, DeltaRotation);
	}
	// 如果 bSinusoidalMovement 为 true，则计算上下浮动。
	if (bSinusoidalMovement)
	{
		// (数学原理): FMath::Sin() 函数接收一个弧度值，返回一个在 [-1, 1] 之间波动的值。
		// RunningTime * SinePeriodConstant 控制了正弦波的频率（波动速度）。
		// SineAmplitude 控制了正弦波的振幅（波动范围）。
		const float Sine = SineAmplitude * FMath::Sin(RunningTime * SinePeriodConstant);
		// 在初始位置的 Z 轴上加上这个波动值，就实现了上下浮动的效果。
		CalculatedLocation = InitialLocation + FVector(0.f, 0.f, Sine);
	}
}

/**
 * @brief 对一个进入效果区域的目标 Actor 应用一个指定的 Gameplay Effect (GE)。
 * @param TargetActor 将要被施加效果的目标 Actor。
 * @param GameplayEffectClass 要施加的 GE 的蓝图类。
 *
 * @par 功能说明
 * 该函数是“效果施加器”Actor（如治疗光环、火焰陷阱）的核心。当一个 Actor（通常是玩家或敌人）
 * 与这个效果 Actor 发生交互（例如，重叠）时，此函数被调用。它负责验证目标、创建并应用 GE，
 * 并根据 GE 的类型和预设的策略来决定后续行为（例如，是否在效果结束后移除它，或者施加效果后自我销毁）。
 *
 * @par 详细流程
 * 1.  **目标过滤**: 检查目标 Actor 是否带有 "Enemy" 标签。如果带了，并且本效果 Actor 的 `bApplyEffectsToEnemies` 标志为 `false`，则直接中断函数，不对其施加效果。这是一种简单的敌我识别机制。
 * 2.  **获取 ASC**: 尝试从 `TargetActor` 获取其 `AbilitySystemComponent` (ASC)。如果目标没有 ASC，则无法对其施加 GE，函数中断。
 * 3.  **创建并应用 GE**:
 *     - 使用与 `ApplyEffectToSelf` 相同的标准流程：创建 `Context` -> 添加 `SourceObject` -> 创建 `Spec` -> `ApplyGameplayEffectSpecToSelf`。
 *     - 注意 `ApplyGameplayEffectSpecToSelf` 在这里是被 `TargetASC` 调用的，所以效果是施加在 `TargetActor` 自己身上。
 * 4.  **处理无限持续时间的 GE**:
 *     - 检查刚刚应用的 GE 的持续时间策略 (`DurationPolicy`) 是否为“无限 (`Infinite`)”。
 *     - 如果是，并且本效果 Actor 的移除策略 (`InfiniteEEffectRemovalPolicy`) 被设置为“在重叠结束时移除 (`RemoveOnEndOverlap`)”，则：
 *     - 将返回的 `ActiveEffectHandle`（这是已应用的 GE 的唯一句柄）和目标的 `ASC` 存储到一个 `TMap` (`ActiveEffectHandles`) 中。
 *     - (为什么这么做？) 这是为了在将来玩家离开这个效果区域时，能够根据这个句柄找到并手动移除这个无限持续的 Buff。
 * 5.  **处理非无限持续时间的 GE**:
 *     - 如果 GE 不是无限时间的（即它是“即时 (`Instant`)”或“有持续时间 (`Has Duration`)”的），并且本效果 Actor 的 `bDestroyOnEffectApplication` 标志为 `true`：
 *     - 调用 `Destroy()` 函数，将效果 Actor 自身销毁。
 *     - (为什么这么做？) 这适用于一次性的效果，比如一个踩上去就爆炸的地雷，或者一个吃掉就消失的药水。
 */
void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	// 步骤 1/5: 过滤目标
	// (为什么这么做): 这是一个简单的阵营过滤。如果目标是敌人，但这个效果被配置为不影响敌人，则直接返回。
	// bApplyEffectsToEnemies 是这个效果 Actor 的一个 UPROPERTY(EditAnywhere) 布尔值。
	if(TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies ) return; 

	// 步骤 2/5: 获取目标的 ASC (Ability System Component)
	// UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent 是获取目标 ASC 的标准静态函数。
	UAbilitySystemComponent * TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	// 如果目标没有 ASC，则无法对它施加效果，直接返回。
	if (TargetASC==nullptr)return;

	// 确保传入的 GE Class 有效，否则在开发版本中使程序崩溃。
	check(GameplayEffectClass)
	// 步骤 3/5: 创建并应用 GE (标准流程)
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext(); // 由目标 ASC 创建上下文。
	
	EffectContextHandle.AddSourceObject(this);// 将本效果 Actor (this) 设为效果来源。
	// 从 GE Class 创建一个效果规格 (Spec)。ActorLevel 是本效果 Actor 的一个成员变量。
	FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass,ActorLevel,EffectContextHandle);
	// 将 Spec 应用到目标 ASC 身上，并返回一个激活效果的句柄 (Handle)。
	FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

	// 步骤 4/5: 处理后续逻辑
	// 检查被应用的 GE 的定义，看它的持续时间策略是否是“无限”。
	const bool bIsInfinite	= EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
	// 如果效果是无限的，并且我们的策略是“重叠结束时移除”...
	if (bIsInfinite && InfiniteEEffectRemovalPolicy==EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		// ...那么就把这个激活效果的句柄和目标的 ASC 存起来。
		// ActiveEffectHandles 是一个 TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*>。
		// 这样，在 OnEndOverlap 事件中，我们就能找到这个 Handle 并移除对应的效果。
		ActiveEffectHandles.Add(ActiveEffectHandle,TargetASC);
	}
	// 如果效果不是无限的（比如是一个瞬时伤害或一个持续10秒的debuff），并且我们设置了“应用后即销毁”...
	if (!bIsInfinite && bDestroyOnEffectApplication)
	{
		// ...那么就销毁自己。这适用于一次性的效果物品，如地雷、药水等。
		Destroy();
	}
}


/**
 * @brief 当有 Actor 与此效果区域发生重叠时调用，根据预设策略应用一种或多种 Gameplay Effect。
 * @param TargetActor 与效果区域发生重叠的目标 Actor。
 *
 * @par 功能说明
 * 这是一个高级别的事件处理函数，它编排了对进入区域的目标应用不同类型 Gameplay Effect (GE) 的逻辑。
 * 这个效果 Actor 被设计为可以同时管理三种不同持续时间的 GE（即时、持续、无限），
 * 并且每种 GE 都有自己的应用策略（例如，是进入时应用，还是离开时应用）。
 * 此函数专门处理“进入时应用 (`ApplyOnOverlap`)”的策略。
 *
 * @par 详细流程
 * 1.  **目标过滤**: 首先，进行敌我识别。如果进入的 `TargetActor` 有 "Enemy" 标签，
 *     但此效果 Actor 的 `bApplyEffectsToEnemies` 标志为 `false`，则忽略该目标，函数直接返回。
 * 2.  **即时效果检查**: 检查 `InstantEffectApplicationPolicy`（即时效果的应用策略）是否被设置为 `ApplyOnOverlap`。
 *     - 如果是，则调用 `ApplyEffectToTarget` 函数，将 `InstantGameplayEffectClass`（预设的即时 GE）应用到目标身上。
 * 3.  **持续效果检查**: 检查 `DurationEffectApplicationPolicy`（持续效果的应用策略）是否被设置为 `ApplyOnOverlap`。
 *     - 如果是，则调用 `ApplyEffectToTarget` 函数，将 `DurationGameplayEffectClass` 应用到目标身上。
 * 4.  **无限效果检查**: 检查 `InfiniteEffectApplicationPolicy`（无限效果的应用策略）是否被设置为 `ApplyOnOverlap`。
 *     - 如果是，则调用 `ApplyEffectToTarget` 函数，将 `InfiniteGameplayEffectClass` 应用到目标身上。
 *
 * @par 注意事项
 * - 此函数的设计高度数据驱动。设计师可以在蓝图中为同一个 `AAuraEffectActor` 配置不同的 GE 和应用策略，
 *   从而快速创建出功能各异的效果区域（例如，一个只造成瞬时伤害的陷阱，一个提供持续治疗的光环，
 *   或一个同时造成瞬时伤害并附加持续中毒效果的区域）。
 * - `InstantEffectApplicationPolicy` 等变量是 `UPROPERTY`，允许在编辑器中通过下拉菜单选择策略。
 */
void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{

	// 步骤 1/4: 过滤掉不应受影响的目标。
	// 这是一个简单的敌我识别逻辑，用于增加效果区域的灵活性。
	if(TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies ) return; 

	// 步骤 2/4: 根据“即时效果”的应用策略，决定是否应用效果。
	// InstantEffectApplicationPolicy 是一个枚举变量，在蓝图中配置。
	// 如果它的值被设为 EEffectApplicationPolicy::ApplyOnOverlap...
	if (InstantEffectApplicationPolicy==EEffectApplicationPolicy::ApplyOnOverlap)
	{
		// ...就调用通用的效果应用函数，传入目标和预设的“即时效果GE类”。
		ApplyEffectToTarget(TargetActor,InstantGameplayEffectClass);
	}

	// 步骤 3/4: 根据“持续效果”的应用策略，决定是否应用效果。
	// 如果它的值被设为 EEffectApplicationPolicy::ApplyOnOverlap...
	if (DurationEffectApplicationPolicy ==EEffectApplicationPolicy::ApplyOnOverlap)
	{
		// ...就应用预设的“持续效果GE类”。
		ApplyEffectToTarget(TargetActor,DurationGameplayEffectClass);
	}

	// 步骤 4/4: 根据“无限效果”的应用策略，决定是否应用效果。
	// 如果它的值被设为 EEffectApplicationPolicy::ApplyOnOverlap...
	if (InfiniteEffectApplicationPolicy ==EEffectApplicationPolicy::ApplyOnOverlap)
	{
		// ...就应用预设的“无限效果GE类”。
		ApplyEffectToTarget(TargetActor,InfiniteGameplayEffectClass);
	}
	
}


/**
 * @brief 当有 Actor 离开此效果区域时调用，根据预设策略应用效果或移除效果。
 * @param TargetActor 离开效果区域的目标 Actor。
 *
 * @par 功能说明
 * 该函数负责处理角色离开效果区域时的所有逻辑。它主要有两个职责：
 * 1.  **应用“离开时触发”的效果**: 与 `OnOverlap` 类似，它会检查是否有任何 GE 的应用策略被设置为 `ApplyOnEndOverlap`，并在此时应用它们。这适用于一些特殊设计，例如“离开治疗光环时获得一个短暂的护盾”。
 * 2.  **移除无限时长的效果**: 这是此函数最核心和最常见的功能。它会检查本效果 Actor 的移除策略是否为 `RemoveOnEndOverlap`。如果是，它就会找到之前在 `OnOverlap` 时施加并记录下来的那个无限时长的 GE（例如“持续治疗”光环效果），并将其从目标身上移除。
 *
 * @par 详细流程
 * 1.  **目标过滤**: 同样，首先进行敌我识别，忽略不应受影响的目标。
 * 2.  **应用“离开时触发”的效果**: 依次检查即时、持续和无限效果的应用策略，如果策略是 `ApplyOnEndOverlap`，则调用 `ApplyEffectToTarget` 应用对应的 GE。
 * 3.  **移除效果检查**: 检查 `InfiniteEEffectRemovalPolicy` 是否为 `RemoveOnEndOverlap`。
 * 4.  **查找并移除**: 如果需要移除：
 *     - 获取目标 Actor 的 ASC。
 *     - 遍历 `ActiveEffectHandles` 这个 TMap，这个 Map 中存储了所有由此 Actor 施加的、需要被移除的无限效果的句柄（Handle）。
 *     - 通过比较 ASC 指针，找到属于当前离开的 `TargetActor` 的所有效果句柄。
 *     - 对找到的每一个句柄，调用 `TargetASC->RemoveActiveGameplayEffect()` 来真正地移除效果。同时，将这个句柄添加到一个临时的待删除数组 `HandlesToRemove` 中。
 * 5.  **清理 TMap**: 在遍历结束后，再次遍历 `HandlesToRemove` 数组，将其中所有的句柄从 `ActiveEffectHandles` TMap 中安全地移除。
 *
 * @par 注意事项
 * - **遍历时修改容器的陷阱**: 代码中使用了两步移除法（先收集再删除），这是**非常重要且正确**的做法。绝不能在 `for (auto HandlePar : ActiveEffectHandles)` 循环内部直接删除 `ActiveEffectHandles` 的元素，因为这会使迭代器失效，导致程序崩溃或未定义行为。
 * - **Bug 警告**: 在 `InfiniteEffectApplicationPolicy` 的检查中，似乎有一个复制粘贴错误，它检查的是 `ApplyOnOverlap` 而不是 `ApplyOnEndOverlap`。
 */
void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	// 步骤 1/4: 过滤目标，与 OnOverlap 逻辑一致。
	if(TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies ) return; 

	// 步骤 2/4: 应用那些被配置为“离开时触发”的效果。
	// 这部分逻辑与 OnOverlap 类似，只是检查的策略是 ApplyOnEndOverlap。
	if (InstantEffectApplicationPolicy==EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor,InstantGameplayEffectClass);
	}
	
	if (DurationEffectApplicationPolicy ==EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor,DurationGameplayEffectClass);
	}

	if (InfiniteEffectApplicationPolicy ==EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor,InfiniteGameplayEffectClass);
	}

	// 步骤 3/4: 处理“离开时移除”无限效果的逻辑。
	if (InfiniteEEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		// 获取目标 ASC。
		UAbilitySystemComponent * TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (!IsValid(TargetASC))return;
		
		// (为什么用两步删除法): 直接在 `for (auto& Pair : MyMap)` 循环中调用 `MyMap.Remove(Pair.Key)`
		// 会使迭代器失效，导致崩溃。正确的做法是：先遍历一遍，把要删除的 Key 收集起来...
		TArray<FActiveGameplayEffectHandle> HandlesToRemove;// 创建一个临时数组，用于收集要删除的 Key。

		// `ActiveEffectHandles` 是一个 TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*>
		// 它存储了所有由这个 Actor 施加的、且需要被移除的无限效果。
		for (auto HandlePar : ActiveEffectHandles)
		{
			// 检查当前遍历到的效果，其目标 ASC 是否就是刚刚离开的这个 TargetASC。
			if (TargetASC == HandlePar.Value)
			{
				// 如果是，就调用 ASC 的函数，使用效果句柄 (HandlePair.Key) 来移除这个 GE。
				// 第二个参数 `1` 表示移除 1 层堆叠（对于不可堆叠的效果，这会完全移除它）。
				TargetASC->RemoveActiveGameplayEffect(HandlePar.Key,1);
				// 将这个句柄（Key）添加到我们的待删除列表中。
				HandlesToRemove.Add(HandlePar.Key);
			}
		}
		// 步骤 4/4: 清理 TMap
		// ...然后在第一个循环结束后，再安全地从 TMap 中移除这些 Key。
		for (auto Handler : HandlesToRemove)
		{
			// FindAndRemoveChecked 假设 Key 一定存在，如果不存在会崩溃。
			// 在这个逻辑中是安全的，因为 Key 就是从 TMap 本身收集来的。
			ActiveEffectHandles.FindAndRemoveChecked(Handler);// 使用 Remove 比 FindAndRemoveChecked 更安全一些
		}
		
	}
}


