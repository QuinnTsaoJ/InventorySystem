// 背包网格控件 —— 格子渲染、坐标转换、拖拽放置、放置预览
// InventorySystem Plugin

#pragma once

#include "CoreMinimal.h"
#include "GridBackgroundWidget.h"
#include "Blueprint/UserWidget.h"
#include "InventoryGridWidget.generated.h"

class UCanvasPanel;
class UCanvasPanelSlot;
class UInventoryComponent;
class UInventoryItemWidget;
class UDragDropOperation;
class UInventoryDragDropOperation;
class UInventoryPanelWidget;
class UInventoryTooltipWidget;
struct FInventoryItemInstance;

UCLASS()
class INVENTORYUI_API UInventoryGridWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	//---------------------------- 初始化 ---------------------------

	/** 绑定背包组件，绑定委托，首次刷新 */
	void InitializeGrid(UInventoryComponent* InInventory);

	/** 设置所属面板引用，用于右键菜单等向上传递事件 */
	void SetParentPanel(UInventoryPanelWidget* InPanel);

	/** 设置工具提示控件引用，传递给 ItemWidget 用于悬浮显示 */
	void SetTooltipWidget(UInventoryTooltipWidget* InTooltip);

	/** ItemWidget 右键菜单回调：转发到 Panel 的 Blueprint 事件 */
	void OnItemContextMenu(const FGuid& ItemID, const FVector2D& ScreenPosition);

	//---------------------------- 全面刷新 ---------------------------

	/** 清除失效控件、更新现存控件、创建缺失控件 */
	void RefreshAllItems();
	
	void ClearInventory();

	//---------------------------- 坐标转换 ---------------------------

	/** 格子坐标 → 像素坐标（左上角） */
	FVector2D GridToPixel(FIntPoint GridPos) const;

	/** 像素坐标（相对本控件） → 格子坐标 */
	FIntPoint PixelToGrid(FVector2D PixelPos) const;

	//---------------------------- 配置 ---------------------------

	/** 单个格子的像素尺寸 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float CellSize = 64.f;

	/** 网格线粗细（像素） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float GridLineThickness = 2.f;

	/** ItemWidget 蓝图类，用于动态创建物品控件 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSubclassOf<UInventoryItemWidget> ItemWidgetClass;

protected:
	//---------------------------- 生命周期 ---------------------------

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	//---------------------------- 自定义绘制 ---------------------------

	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	//---------------------------- 拖拽事件 ---------------------------

	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	//---------------------------- 控件树 ---------------------------

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> ItemCanvas;
	
	//---------------------------- 内部状态 ---------------------------

	UPROPERTY()
	TWeakObjectPtr<UInventoryComponent> Inventory;

	/** InstanceID → ItemWidget 映射 */
	UPROPERTY()
	TMap<FGuid, UInventoryItemWidget*> ItemWidgets;

	/** 格子背景控件（独立子控件，绘制黑底+网格线，ZOrder 最低） */
	UPROPERTY()
	TObjectPtr<UGridBackgroundWidget> GridBackground;

	/** 所属面板弱引用，用于右键菜单等向上传递事件 */
	TWeakObjectPtr<UInventoryPanelWidget> ParentPanel;

	/** 工具提示控件弱引用，传递给 ItemWidget */
	TWeakObjectPtr<UInventoryTooltipWidget> TooltipWidget;

	//---------------------------- 拖拽预览状态 ---------------------------

	/** 当前正在拖拽的物品 ID（无效时表示无拖拽） */
	FGuid DragItemID;

	/** 预览色块的目标格子坐标 */
	FIntPoint DragPreviewGridPos;

	/** 预览色块的格子尺寸（宽×高，考虑旋转） */
	FIntPoint DragPreviewSize;

	/** 当前拖拽物品的旋转状态 */
	bool DragItemRotated = false;

	/** 拖拽物品的来源背包（跨背包拖拽时与本网格的 Inventory 不同） */
	TWeakObjectPtr<UInventoryComponent> DragSourceInventory;

	/** 拖拽期间隐藏的原物品控件，仅在拖拽真正结束时恢复显示 */
	TWeakObjectPtr<UInventoryItemWidget> DragSourceItemWidget;

	//---------------------------- 拖拽委托句柄 ---------------------------

	FDelegateHandle OnDragRotatedHandle;
	bool bDragCancelledBound = false;
	FGeometry CachedDragGeometry;

	//---------------------------- 背包委托解绑标记 ---------------------------

	bool bInventoryDelegatesBound = false;

	//---------------------------- 内部辅助 ---------------------------

	UInventoryItemWidget* CreateItemWidget(const FInventoryItemInstance& Item);
	void RemoveItemWidget(const FGuid& InstanceID);
	void UpdateItemWidget(const FGuid& InstanceID);
	void UnbindDragDelegates();

	/** 清除拖拽预览状态（不恢复原物品控件可见性） */
	void ClearDragPreview();

	/** 恢复拖拽期间隐藏的原物品控件可见性 */
	void RestoreDragSourceWidget();

	//---------------------------- 委托回调 ---------------------------

	UFUNCTION()
	void OnInventoryItemAdded(const FInventoryItemInstance& Item);
	
	UFUNCTION()
	void OnInventoryItemRemoved(const FGuid& InstanceID);

	UFUNCTION()
	void OnInventoryItemDropped(const FGuid& InstanceID);
	
	UFUNCTION()
	void OnInventoryItemMoved(const FGuid& InstanceID, FIntPoint NewPosition, bool bRotated);
	
	UFUNCTION()
	void OnInventoryItemUpdated(const FGuid& InstanceID);
	UFUNCTION()
	void OnInventoryResized();
	UFUNCTION()
	void OnInventoryCleared();
	void OnDragRotated(UInventoryDragDropOperation* DragOp);
	UFUNCTION()
	void OnDragOperationCancelled(UDragDropOperation* Op);
};
