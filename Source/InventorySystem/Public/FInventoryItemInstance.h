#pragma once

#include "CoreMinimal.h"
#include "DeferredInstancedStruct.h"
#include "FInventoryItemInstance.generated.h"


USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FInventoryItemInstance
{
	GENERATED_BODY()

public:

	// 对应DataTable Row
	/** 物品定义表中的行名 */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, DisplayName = "TableRowID", Category = "Inventory|Instance", meta=(ToolTip="物品定义表中的行名"))
	FName ItemID;

	// 唯一ID
	UPROPERTY(SaveGame)
	FGuid InstanceID;

	// 数量
	/** 当前实例包含的物品数量 */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, DisplayName = "数量", Category = "Inventory|Instance", meta=(ToolTip="当前实例包含的物品数量"))
	int32 Quantity = 1;

	// 左上角位置
	/** 物品在网格中的左上角位置 */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, DisplayName = "左上角位置", Category = "Inventory|Instance", meta=(ToolTip="物品在网格中的左上角位置"))
	FIntPoint Position = FIntPoint::ZeroValue;

	// 是否旋转
	/** 当前实例是否已旋转 */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, DisplayName = "旋转", Category = "Inventory|Instance", meta=(ToolTip="当前实例是否已旋转"))
	bool bRotated = false;

	// 是否允许堆叠（冗余数据，方便运行时读取）
	/** 当前实例是否允许堆叠 */
	UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadOnly, DisplayName = "堆叠", Category = "Inventory|Instance", meta=(ToolTip="当前实例是否允许堆叠"))
	bool bStackable = false;

	// Runtime数据（软路径持久化，存档跨插件结构体类型重启不丢失）
	/** 当前实例独立保存的运行时数据 */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, DisplayName = "Runtime数据", Category = "Inventory|Instance", meta=(ToolTip="当前实例独立保存的运行时数据"))
	FDeferredInstancedStruct RuntimeData;
};
