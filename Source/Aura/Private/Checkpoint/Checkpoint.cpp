// Fill out your copyright notice in the Description page of Project Settings.


#include "Checkpoint/Checkpoint.h"

#include "Components/SphereComponent.h"

ACheckpoint::ACheckpoint(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	

	CheckpointMesh = CreateDefaultSubobject<UStaticMeshComponent>("CheckpointMesh");
	CheckpointMesh->SetupAttachment(GetRootComponent());
	CheckpointMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CheckpointMesh->SetCollisionResponseToAllChannels(ECR_Block);

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetupAttachment(CheckpointMesh);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ACheckpoint::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag(FName("Player")))
	{
		HandleGlowEffects();
	}
}

void ACheckpoint::BeginPlay()
{
	Super::BeginPlay();
	Sphere->OnComponentBeginOverlap.AddDynamic(this,&ACheckpoint::OnSphereOverlap);
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
