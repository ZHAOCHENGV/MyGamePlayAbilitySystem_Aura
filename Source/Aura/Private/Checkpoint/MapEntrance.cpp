// Fill out your copyright notice in the Description page of Project Settings.


#include "Checkpoint/MapEntrance.h"

#include "Components/SphereComponent.h"
#include "Game/AuraGameModeBase.h"
#include "Interation/PlayerInterface.h"
#include "Kismet/GameplayStatics.h"



/**
* @param ObjectInitializer Unreal Engine 提供的对象初始化器。
*/
AMapEntrance::AMapEntrance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Sphere->SetupAttachment(MoveToComponent);
}


/**
 * @brief (HighlightInterface 接口实现) 高亮地图入口。
 *
 * @par 功能说明
 * 与 ACheckpoint 中的实现类似，开启网格体的自定义深度渲染，以便后期处理材质为其添加描边效果。
 * 与 ACheckpoint 不同的是，这里没有 `if (!bReached)` 的检查。
 *
 * @par 为什么没有检查？
 * 因为地图入口通常是一个永久性的交互点，即使玩家刚刚通过它传送过一次，
 * 再次靠近时它仍然应该高亮，因为它依然可用。它的状态不会像一次性的检查点那样被“消耗”掉。
 */
void AMapEntrance::HighlightActor_Implementation()
{
	// 直接开启自定义深度渲染，无需检查 bReached 状态。
	CheckpointMesh->SetRenderCustomDepth(true);
}



/**
 * @brief 当玩家进入地图入口的触发区域时调用。
 *
 * @par 功能说明
 * 这是地图入口的核心逻辑。当玩家走进触发范围时，它会：
 * 1.  立即保存当前世界的状态。
 * 2.  更新玩家的存档信息，特别是设置下个关卡的出生点 Tag。
 * 3.  触发关卡切换，将玩家传送到目标地图。
 */
void AMapEntrance::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 检查触发者是否为玩家。
	if (OtherActor->Implements<UPlayerInterface>())
	{
		// 步骤 1/4: 标记为已触及（虽然在此类中可能作用不大，但继承自父类）
		bReached = true;
		// 步骤 2/4: 保存当前世界的状态。
		if (AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{
			// 调用 GameMode 的函数来保存当前世界的状态。
			// DestinationMap.ToSoftObjectPath().GetAssetName() 是为了获取目标地图的名称，
			// 这可能是一个特殊版本的 SaveWorldState，用于在切换地图前进行特定的保存操作。
			// **注意**: 这里的 SaveWorldState 函数签名似乎与之前的不同，多了一个地图名参数。
			AuraGameMode->SaveWorldState(GetWorld(),DestinationMap.ToSoftObjectPath().GetAssetName());
		}
		// 步骤 3/4: 更新玩家存档中的下一个出生点信息。
		// DestinationPlayerStartTag 是这个地图入口的一个 UPROPERTY，在蓝图中设置，
		// 用于告诉 GameMode 玩家在到达新地图后应该从哪个 PlayerStart 出生。
		IPlayerInterface::Execute_SaveProgress(OtherActor,DestinationPlayerStartTag);

		// 步骤 4/4: 切换关卡。
		// UGameplayStatics::OpenLevelBySoftObjectPtr 是切换关卡的推荐方式，
		//因为它使用软引用 (TSoftObjectPtr)，不会在游戏启动时就加载所有可能的目标地图。
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, DestinationMap);
	
	}
}

void AMapEntrance::LoadActor_Implementation()
{
	
}