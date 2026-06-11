// 背包功能函数库实现
// InventorySystem Plugin

#include "InventoryFunctionLibrary.h"
#include "InventoryComponent.h"
#include "FInventoryItemInstance.h"

bool UInventoryFunctionLibrary::TransferItemBetweenInventories(
	UInventoryComponent* SourceInventory,
	UInventoryComponent* TargetInventory,
	const FGuid& ItemID)
{
	if (!SourceInventory || !TargetInventory || SourceInventory == TargetInventory)
	{
		return false;
	}

	const FInventoryItemInstance* Item = SourceInventory->FindItem(ItemID);
	if (!Item)
	{
		return false;
	}

	// 在目标背包找可用位置
	FIntPoint TargetPosition;
	if (!TargetInventory->FindAvailablePosition(*Item, TargetPosition))
	{
		return false;
	}

	const FInventoryOperationResult Result = SourceInventory->TransferItem(ItemID, TargetInventory, TargetPosition);
	return Result.bSuccess;
}

bool UInventoryFunctionLibrary::DropItemFromInventory(
	UInventoryComponent* Inventory,
	const FGuid& ItemID)
{
	if (!Inventory)
	{
		return false;
	}
	const FInventoryOperationResult Result = Inventory->DropItem(ItemID);
	return Result.bSuccess;
}
