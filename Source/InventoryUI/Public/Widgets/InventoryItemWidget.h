// 单个物品控件 —— 只存 InstanceID 和弱引用，不缓存物品数据
// InventorySystem Plugin

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryItemWidget.generated.h"

class UImage;
class UTextBlock;
class UInventoryComponent;
class UInventoryDragDropOperation;
class UInventoryTooltipWidget;
class UInventoryGridWidget;

UCLASS()
class INVENTORYUI_API UInventoryItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 绑定此控件到指定物品实例 */
	void InitializeItem(const FGuid& InItemID, UInventoryComponent* InInventory);

	/** 设置单位格子像素尺寸，供拖拽时计算图标大小 */
	void SetCellSize(float InCellSize) { CellSize = InCellSize; }

	/** 设置所属网格控件引用，用于右键菜单等向上传递事件 */
	void SetOwnerGrid(UInventoryGridWidget* InGrid);

	/** 设置工具提示控件引用，用于悬浮显示物品信息 */
	void SetTooltipWidget(UInventoryTooltipWidget* InTooltip);

	/** 从 Runtime 重新拉取最新数据并刷新显示 */
	void Refresh();

	/** 从 Inventory 查询当前物品数据，可能返回 nullptr */
	const struct FInventoryItemInstance* GetItem() const;

	/** 当前绑定的物品 InstanceID */
	FGuid GetItemID() const { return ItemInstanceID; }

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StackCount;

	/** 绑定的物品 InstanceID */
	UPROPERTY()
	FGuid ItemInstanceID;

	/** 所属背包组件弱引用 */
	UPROPERTY()
	TWeakObjectPtr<UInventoryComponent> Inventory;

	/** 单位格子像素尺寸，由 InventoryGridWidget 在创建时设置 */
	float CellSize = 64.f;

	/** 工具提示延迟触发定时器，避免鼠标快速划过时误触发 */
	FTimerHandle TooltipTimerHandle;

	static constexpr float TooltipDelaySeconds = 1.0f;

	/** 清除旧定时器并重新开始工具提示延迟计时 */
	void StartTooltipTimer(const FVector2D& MouseScreenPosition);

private:
	/** 工具提示控件弱引用，悬浮时显示物品详情 */
	TWeakObjectPtr<UInventoryTooltipWidget> TooltipWidget;

	/** 所属网格控件弱引用，用于右键菜单等向上传递事件 */
	TWeakObjectPtr<UInventoryGridWidget> OwnerGrid;
	
	/**记录上一帧鼠标位置*/
	FVector2D PrevMouseScreenPosition;
};
