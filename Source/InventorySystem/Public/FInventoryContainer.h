//目前该结构体未被使用,打算以后将背包功能升级到一个背包管理多个网格时再使用 

#pragma once

#include "CoreMinimal.h"
#include "FInventoryContainer.generated.h"

struct FInventoryItemInstance;

USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FInventoryContainer
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "容器大小")
	FIntPoint GridSize = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "最大承重")
	float MaxWeight = 100.f;

	UPROPERTY(BlueprintReadWrite)
	TArray<int32> OccupancyMap;

	UPROPERTY()
	TArray<FInventoryItemInstance> Items;
};
