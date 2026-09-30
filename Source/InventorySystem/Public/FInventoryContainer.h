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

	/** 容器的网格尺寸 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "容器大小", Category = "Inventory|Container", meta=(ToolTip="容器的网格尺寸"))
	FIntPoint GridSize = FIntPoint::ZeroValue;

	/** 容器允许的最大总重量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "最大承重", Category = "Inventory|Container", meta=(ToolTip="容器允许的最大总重量"))
	float MaxWeight = 100.f;

	/** 每个网格位置的占用状态 */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Container", meta=(ToolTip="每个网格位置的占用状态"))
	TArray<int32> OccupancyMap;

	UPROPERTY()
	TArray<FInventoryItemInstance> Items;
};
