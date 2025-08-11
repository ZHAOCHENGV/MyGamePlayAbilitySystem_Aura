// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/AuraSummonAbility.h"

TArray<FVector> UAuraSummonAbility::GetSpawnLocations()
{
	// 获取角色向前向量
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	// 角色世界坐标
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	// 计算扇形分布参数,每个召唤物之间的角度间隔
	const float DeltaSpread = SpawnSpread / NumMinions;
	//计算扇形分布参数
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-SpawnSpread / 2.f, FVector::UpVector);
	TArray<FVector>SpawnLocations;
	// 为每个召唤物生成位置
	for (int32 i = 0; i < NumMinions; i++)
	{
		// 计算当前方向：从左侧边界开始逐步向右旋转
		const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
		// 计算基础位置（平面位置）
		FVector ChosenSpawnLocation = Location + Direction * FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);
		// 地面检测修正高度（防止浮空）
		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(Hit,ChosenSpawnLocation + FVector(0.0f, 0.0f, 500.0f),ChosenSpawnLocation - FVector(0.0f, 0.0f, 500.0f),ECC_Visibility);
		// 如果检测到地面，使用碰撞点位置
		if (Hit.bBlockingHit)
		{
			ChosenSpawnLocation = Hit.Location;
		}

		//返回生成位置
		SpawnLocations.Add(ChosenSpawnLocation);
		
	}
	
	
	return SpawnLocations;
}

TSubclassOf<APawn> UAuraSummonAbility::GetRandomMinionClass()
{
	//随机获取数字
	const int32 Selection = FMath::RandRange(0, MinionClasses.Num() - 1);
	//获取数组中的Actor
	return MinionClasses[Selection];
}
