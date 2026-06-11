// 背包拖拽操作 —— 存储拖拽过程中的临时状态
// InventorySystem Plugin

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Kismet/KismetSystemLibrary.h"
#include "InventoryDragDropOperation.generated.h"

class UInventoryComponent;

DECLARE_MULTICAST_DELEGATE(FOnDragRotated);

UCLASS()
class INVENTORYUI_API UInventoryDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/** 被拖拽物品的 InstanceID */
	UPROPERTY()
	FGuid ItemID;

	/** 拖拽时物品的旋转状态 */
	UPROPERTY()
	bool bRotated = false;

	/** 鼠标在物品内部的归一化抓取点 (X∈[0,1], Y∈[0,1])，旋转无关 */
	UPROPERTY()
	FVector2D NormalizedGrabPoint;

	/** 拖拽起始时鼠标在图标内的像素偏移，旋转后配合 Offset 补偿位移 */
	UPROPERTY()
	FVector2D DragVisualOffset;

	/** 根据当前物品像素尺寸，计算鼠标到物品左上角的像素偏移 */
	FVector2D GetCurrentDragOffset(FIntPoint ItemPixelSize) const
	{
		return FVector2D(
			NormalizedGrabPoint.X * ItemPixelSize.X,
			NormalizedGrabPoint.Y * ItemPixelSize.Y
		);
	}

	/** 来源背包组件 */
	UPROPERTY()
	TWeakObjectPtr<UInventoryComponent> SourceInventory;

	/** 旋转事件委托（R 键旋转时广播，拖拽目标监听以更新预览） */
	FOnDragRotated OnRotated;

	/** 切换旋转状态，同步更新归一化抓取点，然后广播事件 */
	void RequestRotate()
	{
		// 0° → 90°(CCW): NewX=Y, NewY=1-X
		// 90° → 0°(CW):   NewX=1-Y, NewY=X
		if (bRotated)
			NormalizedGrabPoint = FVector2D(1.0f - NormalizedGrabPoint.Y, NormalizedGrabPoint.X);
		else
			NormalizedGrabPoint = FVector2D(NormalizedGrabPoint.Y, 1.0f - NormalizedGrabPoint.X);

		bRotated = !bRotated;
		OnRotated.Broadcast();
	}
};
