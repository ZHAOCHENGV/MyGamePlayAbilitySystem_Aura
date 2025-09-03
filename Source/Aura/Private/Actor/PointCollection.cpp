// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/PointCollection.h"

#include "GAS/AuraAbilitySystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"


APointCollection::APointCollection()
{
	PrimaryActorTick.bCanEverTick = false;

	Pt_0 = CreateDefaultSubobject<USceneComponent>(TEXT("Pt_0"));
	ImmutablePts.Add(Pt_0);
	SetRootComponent(Pt_0);

	Pt_1 = CreateDefaultSubobject<USceneComponent>(TEXT("Pt_1"));
	ImmutablePts.Add(Pt_1);
	Pt_1 ->SetupAttachment(GetRootComponent());

	Pt_2 = CreateDefaultSubobject<USceneComponent>(TEXT("Pt_2"));
	ImmutablePts.Add(Pt_2);
	Pt_2 ->SetupAttachment(GetRootComponent());

	Pt_3 = CreateDefaultSubobject<USceneComponent>(TEXT("Pt_3"));
	ImmutablePts.Add(Pt_3);
	Pt_3 ->SetupAttachment(GetRootComponent());

	Pt_4 = CreateDefaultSubobject<USceneComponent>(TEXT("Pt_4"));
	ImmutablePts.Add(Pt_4);
	Pt_4 ->SetupAttachment(GetRootComponent());

	Pt_5 = CreateDefaultSubobject<USceneComponent>(TEXT("Pt_5"));
	ImmutablePts.Add(Pt_5);
	Pt_5 ->SetupAttachment(GetRootComponent());

	Pt_6 = CreateDefaultSubobject<USceneComponent>(TEXT("Pt_6"));
	ImmutablePts.Add(Pt_6);
	Pt_6 ->SetupAttachment(GetRootComponent());

	Pt_7 = CreateDefaultSubobject<USceneComponent>(TEXT("Pt_7"));
	ImmutablePts.Add(Pt_7);
	Pt_7 ->SetupAttachment(GetRootComponent());

	Pt_8 = CreateDefaultSubobject<USceneComponent>(TEXT("Pt_8"));
	ImmutablePts.Add(Pt_8);
	Pt_8 ->SetupAttachment(GetRootComponent());

	Pt_9 = CreateDefaultSubobject<USceneComponent>(TEXT("Pt_9"));
	ImmutablePts.Add(Pt_9);
	Pt_9 ->SetupAttachment(GetRootComponent());

	Pt_10 = CreateDefaultSubobject<USceneComponent>(TEXT("Pt_10"));
	ImmutablePts.Add(Pt_10);
	Pt_10 ->SetupAttachment(GetRootComponent());
	

	

}

/**
 * @brief 以 Pt_0 为圆心，按给定偏航角旋转一圈采样点，并用竖直射线把它们“贴地”，返回前 NumPoints 个地面点
 *
 * @param GroundLocation 圆心/地面参考点（当前实现未使用，建议用于兜底或替代 Pt_0 定位；见下方建议）
 * @param NumPoints      需要返回的点数量（必须 ≤ ImmutablePts.Num()）
 * @param YawOverride    围绕世界 Up 轴的偏航旋转角度（单位：度），对除 Pt_0 以外的点生效
 * @return TArray<USceneComponent*>  贴地后的场景点组件数组（当前实现可能多返回 1 个；见“建议 1：越界判断”）
 *
 * 功能说明：
 * - 把不可变点集 ImmutablePts 视为“模板”环形点：以 Pt_0 为中心，其他点相对位移旋转 YawOverride 度；
 * - 对每个点做一条“上下各 500cm 的竖直线段”线性检测（LineTraceSingleByProfile）找地面；
 * - 命中后将该点 Z 置为命中点高度，并用法线对齐 Z 朝向（垂直地面），最后收集进返回数组。
 *
 * 详细流程：
 * 1) 断言 ImmutablePts 数量足够；2) 遍历每个模板点；
 * 3) 非 Pt_0：计算相对 Pt_0 的向量 → 围绕 UpVector 旋转 YawOverride → 设置新世界位置；
 * 4) 以该点为中心，向上 +500/向下 -500 构成一条竖直线段做碰撞查询（忽略附近活体玩家）；
 * 5) 命中则把点 Z 改为 ImpactPoint.Z，并用 ImpactNormal 生成朝上旋转；
 * 6) 收集点到返回数组，直至循环结束。
 *
 * 注意事项：
 * - 线段命中未检查 bBlockingHit 即使用 ImpactPoint/ImpactNormal，命中失败时可能得到“零向量/错误姿态”；见“建议 2”。
 * - 参数 GroundLocation 未被使用，建议作为 Pt_0 缺失时的兜底或替换“以 Pt_0 为圆心”的逻辑；见“建议 3”。
 * - 限制数量逻辑在循环开头用 “> NumPoints” 判断，可能多加 1 个；见“建议 1”。
 * - Trace 使用 Profile "BlockAll"，在多人/地表复杂场景中建议使用“地面专用通道”；见“建议 4”。
 */
TArray<USceneComponent*> APointCollection::GetGroundPoints(const FVector& GroundLocation, int32 NumPoints,float YawOverride)
{
	// 断言模板点数量足够（否则逻辑无意义）——运行时失败会崩溃并提示中文信息
	checkf(ImmutablePts.Num() >= NumPoints, TEXT("试图访问超出范围的地面点"));

	// 收集返回的点；建议后续用 Reserve(NumPoints) 预分配（见建议 5）
	TArray<USceneComponent*> ArrayCopy;

	// 遍历所有模板点（包含 Pt_0）
	for (USceneComponent* Pt : ImmutablePts)
	{
		// 早停：当已收集数量超过目标数量时直接返回（注意此处用 > 可能多收 1 个；见“建议 1”）
		if (ArrayCopy.Num() >= NumPoints) return ArrayCopy;

		// 排除中心点 Pt_0：对非中心点做围绕 Up 轴的偏航旋转
		if (Pt != Pt_0)
		{
			// 计算“从中心点到当前点”的向量
			FVector ToPoint = Pt->GetComponentLocation() - Pt_0->GetComponentLocation();
			// 围绕世界 Up 轴旋转 YawOverride（单位：度）
			ToPoint = ToPoint.RotateAngleAxis(YawOverride, FVector::UpVector);
			// 将当前点放回以中心点为基准的旋转后位置
			Pt->SetWorldLocation(Pt_0->GetComponentLocation() + ToPoint);
		}

		// 从当前点世界位置向上抬高 500cm
		const FVector RaisedLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, Pt->GetComponentLocation().Z + 500.f);
		// 从当前点世界位置向下降低 500cm
		const FVector LoweredLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, Pt->GetComponentLocation().Z - 500.f);

		// 线性检测结果
		FHitResult HitResult;

		// 要忽略的 Actor 列表：周围“活着的玩家”（避免打到玩家身体）
		TArray<AActor*> IgnoreActors;
		UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(this, IgnoreActors, TArray<AActor*>(), 1500.f, GetActorLocation());

		// 碰撞查询参数：设置忽略列表
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActors(IgnoreActors);

		// 执行单次线检测（使用碰撞 Profile：BlockAll；从上向下），命中地面/可阻挡体
		GetWorld()->LineTraceSingleByProfile(HitResult, RaisedLocation, LoweredLocation, FName("BlockAll"), QueryParams);

		// 将位置的 Z 设为命中点的 Z（未检查命中成功；见“建议 2”）
		const FVector AdjustedLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, HitResult.ImpactPoint.Z);
		Pt->SetWorldLocation(AdjustedLocation);

		// 用命中法线对齐组件的 Z 轴（未检查命中成功；见“建议 2”）
		Pt->SetWorldRotation(UKismetMathLibrary::MakeRotFromZ(HitResult.ImpactNormal));

		// 收集该点
		ArrayCopy.Add(Pt);
	}
	// 返回收集的点（可能包含全部模板点；见“建议 1”）
	return ArrayCopy;
}



void APointCollection::BeginPlay()
{
	Super::BeginPlay();
	
}

