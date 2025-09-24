// Fill out your copyright notice in the Description page of Project Settings.


#include "Checkpoint/Checkpoint.h"


#include "Components/SphereComponent.h"
#include "Game/AuraGameModeBase.h"
#include "Interation/PlayerInterface.h"
#include "Kismet/GameplayStatics.h"


ACheckpoint::ACheckpoint(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	

	CheckpointMesh = CreateDefaultSubobject<UStaticMeshComponent>("CheckpointMesh");
	CheckpointMesh->SetupAttachment(GetRootComponent());
	CheckpointMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CheckpointMesh->SetCollisionResponseToAllChannels(ECR_Block);
	CheckpointMesh->SetCustomDepthStencilValue(CustomDepthStencilOverride);
	CheckpointMesh->MarkRenderStateDirty();
		
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetupAttachment(CheckpointMesh);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	MoveToComponent = CreateDefaultSubobject<USceneComponent>("MoveToComponent");
	MoveToComponent->SetupAttachment(GetRootComponent());
}


/**
 * @brief (SaveInterface 接口实现) 在游戏状态被加载时调用，用于恢复检查点的视觉状态。
 *
 * @par 功能说明
 * 这是 `ISaveInterface::LoadActor` 接口的具体实现。当 `GameMode` 的 `LoadWorldState` 函数
 * 成功反序列化完这个检查点 Actor 的数据后，会调用此函数。
 * 它的作用是根据加载回来的 `bReached` 状态来决定是否需要播放检查点已被激活的特效。
 *
 * @par 详细流程
 * 1.  检查 `bReached` 成员变量的值。这个值是在 `LoadWorldState` 中通过 `Serialize` 函数从存档文件中恢复的。
 * 2.  如果 `bReached` 为 `true`，则调用 `HandleGlowEffects()` 函数来显示检查点已被激活的视觉效果（如发光、禁用碰撞等）。
 */
void ACheckpoint::LoadActor_Implementation()
{
	// `bReached` 这个 bool 变量必须在 .h 文件中被 UPROPERTY(SaveGame) 宏标记，
	// 这样在加载时它的值才会被正确地从存档中恢复。
	if (bReached)
	{
		// 如果加载的数据表明这个检查点在保存时已经被触及，
		// 那么就调用效果处理函数，让它在玩家进入关卡时就处于“已激活”的外观状态。
		HandleGlowEffects();
	}
}

/**
 * @brief (HighlightInterface 接口实现) 当需要高亮此检查点时调用，为其开启自定义深度渲染。
 *
 * @par 功能说明
 * 这是 `IHighlightInterface` 中 `HighlightActor` 函数的 C++ 实现 (`_Implementation`)。
 * 当外部系统（例如玩家控制器检测到鼠标悬停）需要让这个检查点高亮时，会调用此函数。
 * 它的核心逻辑是启用 `CheckpointMesh` 组件的“渲染自定义深度” (Render Custom Depth) 功能。
 *
 * @par 渲染自定义深度 (Render Custom Depth)
 * 这是一个特殊的渲染通道。当一个物体的这个选项被开启时，它会额外地被渲染到一个单独的“自定义深度”缓冲中。
 * 后期处理材质 (Post Process Material) 可以读取这个缓冲，并根据其中的信息（通常是深度值）来绘制
 * 各种效果，最常见的就是在物体边缘绘制一个描边（Outline），从而实现高亮。
 *
 * @par 详细流程
 * 1.  **状态检查**: 首先检查 `bReached` 标志。只有当这个检查点**尚未**被激活时，才允许高亮。这是一个很好的游戏性设计，可以防止玩家与已经激活的、无交互意义的物体进行不必要的视觉互动。
 * 2.  **启用效果**: 如果可以高亮，则调用 `CheckpointMesh->SetRenderCustomDepth(true)` 来开启自定义深度渲染。
 *
 * @par 注意事项
 * - 这个函数本身只负责“打开开关”，真正的高亮效果是由一个全局的后期处理材质实现的。
 */
void ACheckpoint::HighlightActor_Implementation()
{
	// 检查这个检查点是否已经被玩家触及 (bReached)。
	// (为什么这么做): 这是一个很好的游戏逻辑判断。如果检查点已经被激活，
	// 它可能就不再是一个可交互的物体了，因此不应该再响应高亮提示，避免误导玩家。
	if (!bReached)
		{
			// SetRenderCustomDepth(true) 是实现描边高亮效果的第一步。
			// 它会告诉渲染器：“把这个 CheckpointMesh 物体也画一份到特殊的‘自定义深度’通道里去。”
			// 之后，一个全局的后期处理材质会检测到这个通道里有东西，并为它画上描边。
			CheckpointMesh->SetRenderCustomDepth(true);
		}
	
}

/**
 * @brief (HighlightInterface 接口实现) 当需要取消高亮此检查点时调用，关闭其自定义深度渲染。
 *
 * @par 功能说明
 * 这是 `IHighlightInterface` 中 `UnHighlightActor` 函数的 C++ 实现。
 * 当鼠标移开或者其他取消高亮的条件满足时，此函数被调用。
 * 它的作用很简单，就是关闭 `CheckpointMesh` 的自定义深度渲染，让后期处理材质不再能“看到”它，
 * 从而移除描边高亮效果。
 */
void ACheckpoint::UnHighlightActor_Implementation()
{
	// 不论检查点是否被激活，当取消高亮时，都应该关闭这个效果，以确保状态一致。
	// 所以这里不需要 if (!bReached) 检查。
	CheckpointMesh->SetRenderCustomDepth(false);
}

/**
 * @brief (接口函数实现) 获取并返回一个预设的“移动目标点”位置。
 * @param OutDestination [out] 一个通过引用传递的 FVector，此函数会将其值修改为目标位置。
 *
 * @par 功能说明
 * 这是某个接口（例如 `IMoveTargetInterface`）中定义的 `SetMoveToLocation` 函数的 C++ 实现。
 * `_Implementation` 后缀表明这是一个 `BlueprintNativeEvent` 的原生 C++ 版本。
 * 该函数的作用是向外部调用者提供一个精确的世界坐标位置。这个位置由附着在此 `ACheckpoint` Actor
 * 上的一个名为 `MoveToComponent` 的组件来定义。
 *
 * @par 使用场景
 * 这个函数通常用于 AI 行为或玩家移动系统中。例如，当玩家激活这个检查点时，一个 NPC 盟友
 * 可能需要移动到检查点旁的一个特定“集合点”进行等待。AI 控制器可以通过接口查询这个检查点，
 * 调用此函数来获取那个精确的集合点坐标，然后命令 NPC 移动过去。
 *
 * @par 详细流程
 * 1.  函数被调用，并传入一个 `FVector` 的引用 `OutDestination`。
 * 2.  获取 `MoveToComponent` 组件的当前世界坐标 (`GetComponentLocation()`)。
 *     - `MoveToComponent` 很可能是一个 `USceneComponent` 或其子类，被设计师在蓝图编辑器中放置在检查点 Actor 的一个相对位置上，以标记这个“移动目标点”。
 * 3.  将获取到的位置坐标赋值给 `OutDestination`。因为 `OutDestination` 是引用传递，这个修改会直接影响到调用方传入的那个 `FVector` 变量。
 *
 * @par 注意事项
 * - **参数是输出 (Out)**: 函数名 `SetMoveToLocation` 可能会让人误以为是“设置”检查点的位置，但从参数 `FVector& OutDestination` 来看，它的实际作用是“设置调用者的目标位置”，是一个 Getter 功能。在 Unreal 中，很多接口函数会通过这种输出参数的方式来“返回”值。
 * - `MoveToComponent` 必须在蓝图或 C++ 构造函数中被正确地创建和附加，否则 `MoveToComponent->GetComponentLocation()` 会导致程序崩溃。
 */
void ACheckpoint::SetMoveToLocation_Implementation(FVector& OutDestination)
{
	OutDestination = MoveToComponent->GetComponentLocation();
}


/**
 * @brief 当有 Actor 进入检查点的触发区域时被调用。
 * @param OverlappedComponent 触发这次重叠的本 Actor 的组件（即 Sphere）。
 * @param OtherActor 进入触发区域的另一个 Actor。
 * @param ... 其他碰撞事件参数。
 *
 * @par 功能说明
 * 这是检查点的核心交互逻辑。当玩家角色走进 `Sphere` 组件的范围时，此函数会被触发。
 * 它负责将检查点标记为“已到达”，触发一次世界状态的保存，并播放激活时的视觉效果。
 *
 * @par 详细流程
 * 1.  **玩家验证**: 检查进入区域的 `OtherActor` 是否实现了 `UPlayerInterface` 接口。这是一种比 `Cast` 更通用的方式来识别“玩家”或任何应被视为玩家的物体。
 * 2.  **设置状态**: 如果是玩家，立即将 `bReached` 标志设为 `true`。
 * 3.  **保存世界状态**: 获取当前的 `GameMode`，并调用其 `SaveWorldState` 函数。这会立即创建一个包含当前检查点状态（`bReached = true`）的“快照”并存盘。
 * 4.  **通知玩家**: 通过 `PlayerInterface` 调用玩家身上的 `SaveProgress` 函数，并把自己（检查点）的 `PlayerStartTag` 传递过去。玩家接到通知后，可能会更新自己的 `GameInstance`，以便下次死亡或加载时能从这个检查点开始。
 * 5.  **播放特效**: 调用 `HandleGlowEffects()` 来播放视觉和音效反馈，并禁用碰撞以防重复触发。
 */
void ACheckpoint::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 步骤 1/4: 确认触发者是玩家
	// (为什么这么做): `Implements<UPlayerInterface>()` 检查 Actor 是否实现了某个接口。
	// 这比直接 `Cast<AMyCharacter>` 更灵活，因为任何实现了这个接口的类（比如玩家、AI队友）都可以触发检查点。
	if (OtherActor->Implements<UPlayerInterface>())
	{
		// 步骤 2/4: 更新自身状态并触发世界保存
		bReached = true; // 标记此检查点已被触及。
		if (AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{

			const UWorld* World = GetWorld();
			//获取地图名称
			FString MapName = World->GetMapName();
			//删除引擎自动生成的名称
			MapName.RemoveFromStart(World->StreamingLevelsPrefix);
			
			// 请求 GameMode 保存整个世界的当前状态。因为我们刚刚修改了 bReached，
			// 所以这次保存会把 `bReached = true` 这个新状态写入磁盘。
			AuraGameMode->SaveWorldState(GetWorld(),MapName);
		}
		
		// 步骤 3/4: 通知玩家更新其出生点
		// `Execute_` 前缀是调用接口函数的标准方式。
		// 将自身的 PlayerStartTag 传递给玩家，玩家的 SaveProgress 函数会负责把它存到 GameInstance 里。
		IPlayerInterface::Execute_SaveProgress(OtherActor,PlayerStartTag);
		
		// 步骤 4/4: 播放视觉特效
		HandleGlowEffects();
	}
}



/**
 * @brief 在 Actor 初始化时调用，用于绑定碰撞事件的委托。
 */
void ACheckpoint::BeginPlay()
{
	Super::BeginPlay();

	if (bBindOverlapCallback)
	{
		// (为什么这么做): 这是 Unreal Engine 中动态绑定委托（Delegate）的标准方式。
		// `OnComponentBeginOverlap` 是 USphereComponent 的一个多播委托。
		// `AddDynamic` 将一个函数（本类的 OnSphereOverlap）绑定到这个委托上。
		// 从此以后，每当 Sphere 组件检测到有物体进入其范围时，引擎就会自动调用我们提供的 `ACheckpoint::OnSphereOverlap` 函数。
		Sphere->OnComponentBeginOverlap.AddDynamic(this,&ACheckpoint::OnSphereOverlap);
	}
	
}


/**
 * @brief 处理检查点被激活时的发光视觉效果。
 *
 * @par 功能说明
 * 当玩家触及检查点时，此函数被调用以提供即时的视觉反馈。它通过创建一个可动态修改的材质实例（MID）
 * 来控制检查点的外观，通常是为了播放一个激活时的发光动画。同时，它会禁用检查点的触发区域，
 * 使其成为一次性触发的事件。
 *
 * @par 详细流程
 * 1.  **禁用碰撞**: 获取 `Sphere` 组件（很可能是一个 `USphereComponent` 用于检测玩家进入）并禁用其碰撞。这可以防止检查点被重复触发。
 * 2.  **创建动态材质实例 (MID)**:
 *     - 获取 `CheckpointMesh`（检查点的可见网格体）上索引为 0 的材质。
 *     - 以该材质为模板，在内存中创建一个动态副本，即“材质动态实例 (Material Instance Dynamic)”。只有 MID 的参数才能在游戏运行时被代码修改。
 * 3.  **应用动态材质**: 将 `CheckpointMesh` 上原来的静态材质替换为刚刚创建的 MID。从此以后，对这个 MID 参数的任何修改都会立刻反映在模型上。
 * 4.  **触发效果动画**: 调用另一个函数 `CheckpointReached`，并将新创建的 MID 作为参数传递过去。这个函数很可能是一个 `BlueprintImplementableEvent` 或 `BlueprintNativeEvent`，允许美术或设计师在蓝图中通过时间轴 (Timeline) 来实现具体的发光动画（例如，在几秒内逐渐增加材质的“自发光强度”参数）。
 *
 * @par 注意事项
 * - 这种“创建并替换 MID”的模式是 Unreal Engine 中在运行时修改 actor 材质参数的标准做法。
 * - 实际的发光动画逻辑被封装在 `CheckpointReached` 函数中，这是一种良好的实践，将“准备工作”和“执行动画”分离开来。
 */
void ACheckpoint::HandleGlowEffects()
{
	// 步骤 1/4: 禁用触发器的碰撞，防止玩家离开再进入时重复触发。
	// Sphere 应该是一个 USphereComponent，用作触发区域。
	Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 步骤 2/4: 创建一个材质动态实例 (MID)，以便在运行时修改其参数。
	// (为什么这么做): 直接修改原始材质会影响场景中所有使用该材质的物体。
	// 创建一个动态实例 (MID) 相当于为这一个特定的 CheckpointMesh 创建了一个专属的、可编辑的材质副本。
	UMaterialInstanceDynamic* DynamicMaterialInstance = UMaterialInstanceDynamic::Create(CheckpointMesh->GetMaterial(0), this);

	// 步骤 3/4: 将网格体的材质替换为我们刚刚创建的 MID。
	CheckpointMesh->SetMaterial(0, DynamicMaterialInstance);

	// 步骤 4/4: 调用另一个函数（在蓝图中实现的事件），并把 MID 传过去，让它来处理具体的发光动画。
	CheckpointReached(DynamicMaterialInstance);
}
