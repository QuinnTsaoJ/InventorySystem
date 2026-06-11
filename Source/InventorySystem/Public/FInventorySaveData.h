// 背包存档数据结构 —— 用于 Save/Load，游戏层负责持久化
// InventorySystem Plugin

#pragma once

#include "CoreMinimal.h"
#include "FInventoryItemInstance.h"
#include "FQuickSlot.h"
#include "FInventorySaveData.generated.h"

USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FInventorySaveData
{
	GENERATED_BODY()

	/** 所有物品实例 */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Inventory|Save")
	TArray<FInventoryItemInstance> Items;

	/** 快捷栏绑定 */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Inventory|Save")
	TArray<FQuickSlot> QuickSlots;

	/** 背包格子尺寸 */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Inventory|Save")
	FIntPoint GridSize = FIntPoint(10, 6);

	/** 最大承重 */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Inventory|Save")
	float MaxWeight = 100.f;
};
