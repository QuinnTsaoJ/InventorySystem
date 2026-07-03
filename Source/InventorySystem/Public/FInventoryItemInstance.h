#pragma once

#include "CoreMinimal.h"
#include "InstancedStruct.h"
#include "StructUtils/InstancedStruct.h"
#include "FInventoryItemInstance.generated.h"


USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FInventoryItemInstance
{
	GENERATED_BODY()

public:

	// 对应DataTable Row
	UPROPERTY(SaveGame,EditAnywhere, BlueprintReadWrite, DisplayName = "TableRowID")
	FName ItemID;

	// 唯一ID
	UPROPERTY(SaveGame)
	FGuid InstanceID;

	// 数量
	UPROPERTY(SaveGame,EditAnywhere, BlueprintReadWrite, DisplayName = "数量")
	int32 Quantity = 1;

	// 左上角位置
	UPROPERTY(SaveGame,EditAnywhere, BlueprintReadWrite, DisplayName = "左上角位置")
	FIntPoint Position = FIntPoint::ZeroValue;

	// 是否旋转
	UPROPERTY(SaveGame,EditAnywhere, BlueprintReadWrite, DisplayName = "旋转")
	bool bRotated = false;

	// 是否允许堆叠（冗余数据，方便运行时读取）
	UPROPERTY(SaveGame,EditDefaultsOnly, BlueprintReadOnly, DisplayName = "堆叠")
	bool bStackable = false;

	// Runtime数据
	UPROPERTY(SaveGame,EditAnywhere, BlueprintReadWrite, DisplayName = "Runtime数据")
	FInstancedStruct RuntimeData;
};
