// 背包功能函数库 —— 跨容器转移等协调逻辑
// 协调层不属于 Runtime，也不属于单个 Component，作为静态工具函数存在
// InventoryUI Module

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventoryFunctionLibrary.generated.h"

class UInventoryComponent;

UCLASS()
class INVENTORYUI_API UInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 在两个背包间转移物品。
	 * 协调层负责：找到物品 → 在目标背包找可用位置 → 调用 Runtime TransferItem
	 *
	 * @param SourceInventory 来源背包
	 * @param TargetInventory 目标背包
	 * @param ItemID 要转移的物品 InstanceID
	 * @return 是否转移成功
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static bool TransferItemBetweenInventories(
		UInventoryComponent* SourceInventory,
		UInventoryComponent* TargetInventory,
		const FGuid& ItemID
	);

	/**
	 * 将物品丢出背包（从背包移除，不做世界生成——世界生成由游戏层实现）
	 *
	 * @param Inventory 背包
	 * @param ItemID 要丢弃的物品 InstanceID
	 * @return 是否移除成功
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static bool DropItemFromInventory(
		UInventoryComponent* Inventory,
		const FGuid& ItemID
	);
};
