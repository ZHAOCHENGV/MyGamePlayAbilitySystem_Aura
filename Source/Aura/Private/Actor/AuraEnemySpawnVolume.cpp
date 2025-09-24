// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraEnemySpawnVolume.h"


#include "Actor/AuraEnemySpawnPoint.h"
#include "Components/BoxComponent.h"
#include "Interation/PlayerInterface.h"


AAuraEnemySpawnVolume::AAuraEnemySpawnVolume()
{
 	
	PrimaryActorTick.bCanEverTick = false;

	// 步骤 1/3: 创建一个 UBoxComponent (盒状碰撞体) 作为触发区域。
	// CreateDefaultSubobject 是在构造函数中创建组件的标准方法。
	Box = CreateDefaultSubobject<UBoxComponent>(FName("Box"));
	// 将 Box 组件设置为此 Actor 的根组件。这意味着 Actor 的位置就是 Box 的位置。
	SetRootComponent(Box);
	// 步骤 2/3: 配置碰撞类型。
	// QueryOnly: 这个盒子只用于检测重叠 (Overlap)，不产生物理碰撞（比如挡住玩家）。
	Box->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	// ECC_WorldStatic: 将这个盒子的对象类型设为“世界静态物体”。
	Box->SetCollisionObjectType(ECC_WorldStatic);
	// 步骤 3/3: 配置碰撞响应。
	// ECR_Ignore: 首先，让它忽略与所有类型的物体的碰撞。
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	// ECR_Overlap: 然后，唯独对 Pawn (玩家、敌人等角色) 类型的物体，将其响应设置为“重叠 (Overlap)”。
	// (为什么这么做): 这种“先全部忽略，再单独开启”的设置方式非常安全和明确，
	// 它确保了这个触发器只会且必须只对 Pawn 产生重叠事件。
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	

}

/**
 * @brief (ISaveInterface 接口实现) 在游戏加载时调用。
 *
 * @par 功能说明
 * 如果加载的存档数据表明这个刷怪区域在保存时已经被触发过 (`bReached` is true)，
 * 那么在加载关卡后，就直接将这个触发器 Actor 销毁。
 * 这可以防止玩家回到一个已经清空的区域时，敌人被重复生成。
 */
void AAuraEnemySpawnVolume::LoadActor_Implementation()
{
	if (bReached) 
	{
		Destroy();
	}
}


void AAuraEnemySpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	// 动态地将本类的 OnBoxOverlap 函数绑定到 Box 组件的 OnComponentBeginOverlap 委托上。
	// 当有物体进入 Box 的范围并满足碰撞响应设置时，OnBoxOverlap 函数就会被自动调用。
	Box->OnComponentBeginOverlap.AddDynamic(this, &AAuraEnemySpawnVolume::OnBoxOverlap);
	
}


/**
 * @brief 当有 Actor 进入 Box 触发区域时被调用。
 */
void AAuraEnemySpawnVolume::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 步骤 1/4: 检查进入者是否为玩家。
	// 使用接口检查，而不是具体的类转换，这更加灵活。
	if (!OtherActor->Implements<UPlayerInterface>()) return;

	// 步骤 2/4: 标记此触发器已被激活。
	// 这个状态将被保存，以便在 LoadActor_Implementation 中使用。
	bReached = true;

	// 步骤 3/4: 激活所有关联的生成点。
	// SpawnPoints 是一个 UPROPERTY 的 TArray<AAuraEnemySpawnPoint*>，在蓝图编辑器中指定。
	for (AAuraEnemySpawnPoint* Point : SpawnPoints)
	{
		// 安全检查，防止数组中包含了无效的或已被销毁的生成点。
		if (IsValid(Point))
		{
			Point->SpawnEnemy();// 命令每个生成点生成它的敌人。
		}
	}
	// 步骤 4/4: 禁用触发器，使其成为一次性事件。
	// 刷怪完成后，立即禁用 Box 的碰撞，防止玩家离开再进入时重复刷怪。
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}



