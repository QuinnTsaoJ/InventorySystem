// 背包网格控件实现
// InventorySystem Plugin

#include "Widgets/InventoryGridWidget.h"
#include "Widgets/InventoryItemWidget.h"
#include "Widgets/InventoryPanelWidget.h"
#include "Tooltip/InventoryTooltipWidget.h"
#include "DragDrop/InventoryDragDropOperation.h"
#include "InventoryComponent.h"
#include "FInventoryItemInstance.h"
#include "FInventoryItemDefinition.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Widgets/GridBackgroundWidget.h"
#include "Styling/SlateStyle.h"

//=============================================================================
// 初始化
//=============================================================================

void UInventoryGridWidget::SetParentPanel(UInventoryPanelWidget* InPanel)
{
	ParentPanel = InPanel;
}

void UInventoryGridWidget::SetTooltipWidget(UInventoryTooltipWidget* InTooltip)
{
	TooltipWidget = InTooltip;
}

void UInventoryGridWidget::InitializeGrid(UInventoryComponent* InInventory)
{
	if (Inventory == InInventory) {
		RefreshAllItems();
		return;
	}
	
	Inventory = InInventory;
	if (!Inventory.IsValid())
	{
		return;
	}

	if (!bInventoryDelegatesBound)
	{
		Inventory->OnItemAdded.AddDynamic(this, &UInventoryGridWidget::OnInventoryItemAdded);
		Inventory->OnItemRemoved.AddDynamic(this, &UInventoryGridWidget::OnInventoryItemRemoved);
		Inventory->OnItemDropped.AddDynamic(this, &UInventoryGridWidget::OnInventoryItemDropped);
		Inventory->OnItemMoved.AddDynamic(this, &UInventoryGridWidget::OnInventoryItemMoved);
		Inventory->OnItemUpdated.AddDynamic(this, &UInventoryGridWidget::OnInventoryItemUpdated);
		Inventory->OnInventoryResized.AddDynamic(this, &UInventoryGridWidget::OnInventoryResized);
		Inventory->OnInventoryCleared.AddDynamic(this, &UInventoryGridWidget::OnInventoryCleared);
		bInventoryDelegatesBound = true;
	}

	if (GridBackground)
	{
		GridBackground->SetGridParameters(Inventory->GridSize, CellSize, GridLineThickness);
	}

	// 将背景控件的 Slot 尺寸设为完整网格像素大小，使 CanvasPanel 撑满整个网格区域，
	// 确保空白格子也能接收拖拽事件
	if (GridBackground)
	{
		const FVector2D GridPixelSize(Inventory->GridSize.X * CellSize, Inventory->GridSize.Y * CellSize);
		if (UCanvasPanelSlot* BgSlot = Cast<UCanvasPanelSlot>(GridBackground->Slot))
		{
			BgSlot->SetSize(GridPixelSize);
			BgSlot->SetAutoSize(false);
		}
	}

	RefreshAllItems();
}

void UInventoryGridWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemCanvas)
	{
		// 设为 Visible，使空白格子区域也能接收拖拽事件
		// 默认 SelfHitTestInvisible 导致鼠标离开物品图标后事件丢失
		ItemCanvas->SetVisibility(ESlateVisibility::Visible);

		// 背景控件（ZOrder 最低，在所有物品下方）
		GridBackground = NewObject<UGridBackgroundWidget>(this);
		if (UCanvasPanelSlot* BgSlot = ItemCanvas->AddChildToCanvas(GridBackground))
		{
			BgSlot->SetZOrder(-100);
			BgSlot->SetAnchors(FAnchors(0, 0, 0, 0));
			BgSlot->SetAutoSize(true);
		}
	}
}

void UInventoryGridWidget::NativeDestruct()
{
	if (bInventoryDelegatesBound && Inventory.IsValid())
	{
		Inventory->OnItemAdded.RemoveDynamic(this, &UInventoryGridWidget::OnInventoryItemAdded);
		Inventory->OnItemRemoved.RemoveDynamic(this, &UInventoryGridWidget::OnInventoryItemRemoved);
		Inventory->OnItemDropped.RemoveDynamic(this, &UInventoryGridWidget::OnInventoryItemDropped);
		Inventory->OnItemMoved.RemoveDynamic(this, &UInventoryGridWidget::OnInventoryItemMoved);
		Inventory->OnItemUpdated.RemoveDynamic(this, &UInventoryGridWidget::OnInventoryItemUpdated);
		Inventory->OnInventoryResized.RemoveDynamic(this, &UInventoryGridWidget::OnInventoryResized);
		Inventory->OnInventoryCleared.RemoveDynamic(this, &UInventoryGridWidget::OnInventoryCleared);
		bInventoryDelegatesBound = false;
	}

	UnbindDragDelegates();
	ClearDragPreview();
	RestoreDragSourceWidget();

	for (auto& Pair : ItemWidgets)
	{
		if (Pair.Value)
		{
			Pair.Value->RemoveFromParent();
		}
	}
	ItemWidgets.Empty();

	Super::NativeDestruct();
}

//=============================================================================
// 刷新
//=============================================================================

void UInventoryGridWidget::RefreshAllItems()
{
	if (!(Inventory.IsValid()&&IsVisible()))
	{
		return;
	}

	const TArray<FInventoryItemInstance>& Items = Inventory->GetItems();

	TSet<FGuid> CurrentIDs;
	for (const FInventoryItemInstance& Item : Items)
	{
		CurrentIDs.Add(Item.InstanceID);
	}

	TArray<FGuid> ToRemove;
	for (const auto& Pair : ItemWidgets)
	{
		if (!CurrentIDs.Contains(Pair.Key))
		{
			ToRemove.Add(Pair.Key);
		}
	}
	for (const FGuid& ID : ToRemove)
	{
		RemoveItemWidget(ID);
	}

	for (const FInventoryItemInstance& Item : Items)
	{
		if ( ItemWidgets.Find(Item.InstanceID))
		{
			UpdateItemWidget(Item.InstanceID);
		}
		else
		{
			CreateItemWidget(Item);
		}
	}
}

void UInventoryGridWidget::ClearInventory(){
	if (ItemCanvas) {
		for (auto Element : ItemWidgets) {
			Element.Value->RemoveFromParent();
		}
	}
	
	ItemWidgets.Empty();
	
	if (bInventoryDelegatesBound && Inventory.IsValid())
	{
		Inventory->OnItemAdded.RemoveDynamic(this, &UInventoryGridWidget::OnInventoryItemAdded);
		Inventory->OnItemRemoved.RemoveDynamic(this, &UInventoryGridWidget::OnInventoryItemRemoved);
		Inventory->OnItemDropped.RemoveDynamic(this, &UInventoryGridWidget::OnInventoryItemDropped);
		Inventory->OnItemMoved.RemoveDynamic(this, &UInventoryGridWidget::OnInventoryItemMoved);
		Inventory->OnItemUpdated.RemoveDynamic(this, &UInventoryGridWidget::OnInventoryItemUpdated);
		Inventory->OnInventoryResized.RemoveDynamic(this, &UInventoryGridWidget::OnInventoryResized);
		Inventory->OnInventoryCleared.RemoveDynamic(this, &UInventoryGridWidget::OnInventoryCleared);
		bInventoryDelegatesBound = false;
	}

	UnbindDragDelegates();
	ClearDragPreview();
	RestoreDragSourceWidget();
	Inventory.Reset();
}

//=============================================================================
// 坐标转换
//=============================================================================

FVector2D UInventoryGridWidget::GridToPixel(FIntPoint GridPos) const
{
	return FVector2D(GridPos.X * CellSize, GridPos.Y * CellSize);
}

FIntPoint UInventoryGridWidget::PixelToGrid(FVector2D PixelPos) const
{
	// RoundToInt 使预览色块在越过格子中点的瞬间切换，左右上下完全对称
	return FIntPoint(
		FMath::RoundToInt32(PixelPos.X / CellSize),
		FMath::RoundToInt32(PixelPos.Y / CellSize)
	);
}

//=============================================================================
// 自定义绘制：网格线 + 放置预览色块
//=============================================================================

int32 UInventoryGridWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// 先渲染子控件（背景、物品等）
	LayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	// 绘制放置预览色块
	if (!DragItemID.IsValid() || !Inventory.IsValid())
	{
		return LayerId;
	}

	// 检测是否完全超出背包格子范围（任一方向完全在外面则不渲染）
	if (DragPreviewGridPos.X < 0 ||
		DragPreviewGridPos.Y < 0 ||
		DragPreviewGridPos.X + DragPreviewSize.X > Inventory->GridSize.X ||
		DragPreviewGridPos.Y + DragPreviewSize.Y > Inventory->GridSize.Y)
	{
		return LayerId;
	}

	// 判断合法性：从来源背包获取物品数据，但用当前网格的 Inventory 做放置检测
	UInventoryComponent* SourceInv = DragSourceInventory.IsValid() ? DragSourceInventory.Get() : Inventory.Get();
	const FInventoryItemInstance* Item = SourceInv ? SourceInv->FindItem(DragItemID) : nullptr;
	bool bValid = false;
	if (Item)
	{
		FInventoryItemInstance PreviewItem = *Item;
		PreviewItem.bRotated = DragItemRotated;

		// 仅当来源即当前背包时忽略自身占用；跨背包拖拽无需忽略
		int32 IgnoreIndex = -1;
		if (SourceInv == Inventory.Get())
		{
			const TArray<FInventoryItemInstance>& Items = Inventory->GetItems();
			for (int32 i = 0; i < Items.Num(); ++i)
			{
				if (Items[i].InstanceID == DragItemID)
				{
					IgnoreIndex = i;
					break;
				}
			}
		}

		bValid = Inventory->CanPlaceItem(PreviewItem, DragPreviewGridPos, IgnoreIndex);
	}

	const FLinearColor Color = bValid
		? FLinearColor(0.0f, 1.0f, 0.0f, 0.3f)
		: FLinearColor(1.0f, 0.0f, 0.0f, 0.3f);

	const FVector2D PixelPos = GridToPixel(DragPreviewGridPos);
	const FVector2D PixelSize(DragPreviewSize.X * CellSize, DragPreviewSize.Y * CellSize);

	FSlateDrawElement::MakeBox(
		OutDrawElements, LayerId,
		AllottedGeometry.ToPaintGeometry(FVector2f(PixelSize), FSlateLayoutTransform(FVector2f(PixelPos))),
		FCoreStyle::Get().GetBrush("WhiteBrush"),
		ESlateDrawEffect::None, Color
	);
	return LayerId;
}

//=============================================================================
// 拖拽事件
//=============================================================================

void UInventoryGridWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UInventoryDragDropOperation* DragOp = Cast<UInventoryDragDropOperation>(InOperation);
	if (!DragOp || !Inventory.IsValid())
	{
		return;
	}

	CachedDragGeometry = InGeometry;

	// 存储拖拽物品的信息
	DragItemID = DragOp->ItemID;
	DragItemRotated = DragOp->bRotated;

	// 计算物品格子尺寸（考虑旋转），同时保存来源背包供后续 NativePaint/OnDragRotated 使用
	UInventoryComponent* SourceInv = DragOp->SourceInventory.IsValid() ? DragOp->SourceInventory.Get() : Inventory.Get();
	DragSourceInventory = SourceInv;
	if (const FInventoryItemInstance* Item = SourceInv ? SourceInv->FindItem(DragOp->ItemID) : nullptr)
	{
		FInventoryItemInstance TempItem = *Item;
		TempItem.bRotated = DragOp->bRotated;
		DragPreviewSize = SourceInv->GetItemSize(TempItem);
	}

	// 绑定旋转委托
	if (!OnDragRotatedHandle.IsValid())
	{
		OnDragRotatedHandle = DragOp->OnRotated.AddUObject(this, &UInventoryGridWidget::OnDragRotated, DragOp);
	}

	// 绑定取消委托 —— 仅首次进入时绑定，避免重复添加（AddDynamic 不去重）
	if (!bDragCancelledBound)
	{
		DragOp->OnDragCancelled.AddDynamic(this, &UInventoryGridWidget::OnDragOperationCancelled);
		bDragCancelledBound = true;
	}

	// 同背包拖拽时隐藏原物品控件：拖拽全程隐藏，仅 Drop/Cancelled 时恢复
	if (DragOp->SourceInventory.Get() == Inventory.Get())
	{
		if (UInventoryItemWidget** WidgetPtr = ItemWidgets.Find(DragOp->ItemID))
		{
			DragSourceItemWidget = *WidgetPtr;
			if (DragSourceItemWidget.IsValid())
			{
				DragSourceItemWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}

	Invalidate(EInvalidateWidgetReason::Paint);
}

bool UInventoryGridWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UInventoryDragDropOperation* DragOp = Cast<UInventoryDragDropOperation>(InOperation);
	if (!DragOp || !Inventory.IsValid())
	{
		return false;
	}

	// 拖拽即将结束，解绑所有拖拽委托
	UnbindDragDelegates();

	const FVector2D LocalMouse = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
	const FIntPoint ItemPixelSize(DragPreviewSize.X * CellSize, DragPreviewSize.Y * CellSize);
	const FIntPoint TargetGrid = PixelToGrid(LocalMouse - DragOp->GetCurrentDragOffset(ItemPixelSize));

	FInventoryOperationResult Result;

	if (DragOp->SourceInventory.IsValid() && DragOp->SourceInventory != Inventory)
	{
		Result = DragOp->SourceInventory->TransferItem(DragOp->ItemID, Inventory.Get(), TargetGrid, DragOp->bRotated);
	}
	else
	{
		Result = Inventory->MoveItem(DragOp->ItemID, TargetGrid, DragOp->bRotated);
	}

	// 拖拽结束：恢复原物品控件可见性，清除预览状态
	bDragCancelledBound = false;
	RestoreDragSourceWidget();
	ClearDragPreview();

	return Result.bSuccess;
}

bool UInventoryGridWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UInventoryDragDropOperation* DragOp = Cast<UInventoryDragDropOperation>(InOperation);
	if (!DragOp || !Inventory.IsValid())
	{
		return false;
	}
	CachedDragGeometry = InGeometry;

	const FVector2D LocalMouse = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
	const FIntPoint ItemPixelSize(DragPreviewSize.X * CellSize, DragPreviewSize.Y * CellSize);

	DragPreviewGridPos = PixelToGrid(LocalMouse - DragOp->GetCurrentDragOffset(ItemPixelSize));

	return true;
}

void UInventoryGridWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// 鼠标离开时只解绑旋转委托并清除预览状态
	// 不恢复原物品控件可见性，不解除 OnDragCancelled 绑定 ——
	// 这样鼠标移到另一个网格后 ESC 取消时，本网格仍能收到回调恢复控件
	if (OnDragRotatedHandle.IsValid())
	{
		OnDragRotatedHandle.Reset();
	}
	ClearDragPreview();
}

//=============================================================================
// 拖拽预览
//=============================================================================

void UInventoryGridWidget::ClearDragPreview()
{
	DragItemID = FGuid();
	DragPreviewGridPos = FIntPoint::ZeroValue;
	DragPreviewSize = FIntPoint::ZeroValue;
	DragItemRotated = false;
	DragSourceInventory.Reset();
	Invalidate(EInvalidateWidgetReason::Paint);
}

void UInventoryGridWidget::RestoreDragSourceWidget()
{
	if (DragSourceItemWidget.IsValid() && DragSourceItemWidget->GetParent())
	{
		DragSourceItemWidget->SetVisibility(ESlateVisibility::Visible);
	}
	DragSourceItemWidget.Reset();
}

void UInventoryGridWidget::UnbindDragDelegates()
{
	if (OnDragRotatedHandle.IsValid())
	{
		OnDragRotatedHandle.Reset();
	}
}

void UInventoryGridWidget::OnDragRotated(UInventoryDragDropOperation* DragOp)
{
	if (!DragOp || !Inventory.IsValid())
	{
		return;
	}

	DragItemRotated = DragOp->bRotated;

	UInventoryComponent* SourceInv = DragSourceInventory.IsValid() ? DragSourceInventory.Get() : Inventory.Get();
	const FInventoryItemInstance* Item = SourceInv ? SourceInv->FindItem(DragOp->ItemID) : nullptr;
	if (Item)
	{
		FInventoryItemInstance TempItem = *Item;
		TempItem.bRotated = DragOp->bRotated;
		DragPreviewSize = SourceInv->GetItemSize(TempItem);
	}

	// 同步旋转拖拽预览图标：交换尺寸 + 切换预旋转纹理 + Offset 补偿位移
	if (UImage* DragIcon = Cast<UImage>(DragOp->DefaultDragVisual))
	{
		const FInventoryItemDefinition* Def = SourceInv->GetItemDefinition(Item->ItemID);
		if (Def && Def->Icon)
		{
			UTexture2D* NewIcon = DragOp->bRotated && Def->IconRotated ? Def->IconRotated : Def->Icon;
			const FVector2D NewImageSize = DragOp->bRotated
				? FVector2D(Def->Size.Y * CellSize, Def->Size.X * CellSize)
				: FVector2D(Def->Size.X * CellSize, Def->Size.Y * CellSize);

			FSlateBrush Brush = DragIcon->GetBrush();
			Brush.SetResourceObject(NewIcon);
			Brush.ImageSize = NewImageSize;
			DragIcon->SetBrush(Brush);

			// MouseDown pivot: 图标左上 = 鼠标 - DragVisualOffset + Offset
			// 目标: 图标左上 = 鼠标 - GetCurrentDragOffset(旋转后尺寸)
			const FIntPoint NewItemPixelSize(DragPreviewSize.X * CellSize, DragPreviewSize.Y * CellSize);
			const FVector2D PixelDiff = DragOp->DragVisualOffset - DragOp->GetCurrentDragOffset(NewItemPixelSize);
				DragOp->Offset = FVector2D(PixelDiff.X / NewItemPixelSize.X, PixelDiff.Y / NewItemPixelSize.Y);
		}
	}

	//Invalidate(EInvalidateWidgetReason::Paint);
}

void UInventoryGridWidget::OnDragOperationCancelled(UDragDropOperation* Op)
{
	// 拖拽被中断（ESC 等）：恢复原物品控件可见性，清除预览状态
	bDragCancelledBound = false;
	RestoreDragSourceWidget();
	ClearDragPreview();
}

void UInventoryGridWidget::OnItemContextMenu(const FGuid& ItemID, const FVector2D& ScreenPosition)
{
	if (ParentPanel.IsValid())
	{
		ParentPanel->OnItemContextRequested(Inventory.Get(),ItemID, ScreenPosition);
	}
}

//=============================================================================
// 物品控件管理
//=============================================================================

UInventoryItemWidget* UInventoryGridWidget::CreateItemWidget(const FInventoryItemInstance& Item)
{
	if (!ItemWidgetClass || !ItemCanvas)
	{
		return nullptr;
	}

	UInventoryItemWidget* Widget = CreateWidget<UInventoryItemWidget>(GetOwningPlayer(), ItemWidgetClass);
	if (!Widget)
	{
		return nullptr;
	}

	Widget->InitializeItem(Item.InstanceID, Inventory.Get());
	Widget->SetCellSize(CellSize);
	Widget->SetOwnerGrid(this);
	Widget->SetTooltipWidget(TooltipWidget.Get());

	if (UCanvasPanelSlot* CanvasSlot = ItemCanvas->AddChildToCanvas(Widget))
	{
		const FIntPoint Size = Inventory->GetItemSize(Item);
		CanvasSlot->SetPosition(GridToPixel(Item.Position));
		CanvasSlot->SetSize(FVector2D(Size.X * CellSize, Size.Y * CellSize));
		CanvasSlot->SetZOrder(0);
	}

	ItemWidgets.Add(Item.InstanceID, Widget);
	return Widget;
}

void UInventoryGridWidget::RemoveItemWidget(const FGuid& InstanceID)
{
	if (UInventoryItemWidget** WidgetPtr = ItemWidgets.Find(InstanceID))
	{
		if (*WidgetPtr)
		{
			(*WidgetPtr)->RemoveFromParent();
		}
		ItemWidgets.Remove(InstanceID);
	}
}

void UInventoryGridWidget::UpdateItemWidget(const FGuid& InstanceID)
{
	UInventoryItemWidget** WidgetPtr = ItemWidgets.Find(InstanceID);
	if (!WidgetPtr || !(*WidgetPtr) || !Inventory.IsValid())
	{
		return;
	}

	UInventoryItemWidget* Widget = *WidgetPtr;
	const FInventoryItemInstance* Item = Inventory->FindItem(InstanceID);
	if (!Item)
	{
		RemoveItemWidget(InstanceID);
		return;
	}

	Widget->Refresh();

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot))
	{
		const FIntPoint Size = Inventory->GetItemSize(*Item);
		CanvasSlot->SetPosition(GridToPixel(Item->Position));
		CanvasSlot->SetSize(FVector2D(Size.X * CellSize, Size.Y * CellSize));
	}
}

//=============================================================================
// 委托回调
//=============================================================================

void UInventoryGridWidget::OnInventoryItemAdded(const FInventoryItemInstance& Item)
{
	RefreshAllItems();
}

void UInventoryGridWidget::OnInventoryItemRemoved(const FGuid& InstanceID)
{
	RefreshAllItems();
}

void UInventoryGridWidget::OnInventoryItemDropped(const FGuid& InstanceID)
{
	RefreshAllItems();
}

void UInventoryGridWidget::OnInventoryItemMoved(const FGuid& InstanceID, FIntPoint NewPosition, bool bRotated)
{
	UpdateItemWidget(InstanceID);
}

void UInventoryGridWidget::OnInventoryItemUpdated(const FGuid& InstanceID)
{
	UpdateItemWidget(InstanceID);
}

void UInventoryGridWidget::OnInventoryResized()
{
	if (GridBackground)
	{
		GridBackground->SetGridParameters(Inventory->GridSize, CellSize, GridLineThickness);
	}

	// 同步更新背景控件 Slot 尺寸
	if (GridBackground)
	{
		const FVector2D GridPixelSize(Inventory->GridSize.X * CellSize, Inventory->GridSize.Y * CellSize);
		if (UCanvasPanelSlot* BgSlot = Cast<UCanvasPanelSlot>(GridBackground->Slot))
		{
			BgSlot->SetSize(GridPixelSize);
		}
	}

	RefreshAllItems();
}

void UInventoryGridWidget::OnInventoryCleared()
{
	RefreshAllItems();
}
