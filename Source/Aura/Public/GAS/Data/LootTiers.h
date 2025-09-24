

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LootTiers.generated.h"

USTRUCT(BlueprintType)
struct FLootItem
{
	GENERATED_BODY()

	//战利品类
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="LootTiers|Spawning")
	TSubclassOf<AActor> LootClass;

	//生成几率
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="LootTiers|Spawning")
	float ChanceToSpawn = 0.f;

	//要生成的最大数量
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="LootTiers|Spawning")
	int32 MaxNumberToSpawn = 0.f;

	// 战利品等级覆盖
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="LootTiers|Spawning")
	bool bLootLevelOverride = true;
};



/**
 * 
 */
UCLASS()
class AURA_API ULootTiers : public UDataAsset
{
	GENERATED_BODY()
	
};
