// 单个物品控件实现
// InventorySystem Plugin

#include "Widgets/InventoryItemWidget.h"
#include "Widgets/InventoryGridWidget.h"
#include "Tooltip/InventoryTooltipWidget.h"
#include "DragDrop/InventoryDragDropOperation.h"
#include "InventoryComponent.h"
#include "FInventoryItemDefinition.h"
#include "FInventoryItemInstance.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UInventoryItemWidget::InitializeItem(const FGuid& InItemID, UInventoryComponent* InInventory)
{
	ItemInstanceID = InItemID;
	Inventory = InInventory;
	Refresh();
}

void UInventoryItemWidget::SetOwnerGrid(UInventoryGridWidget* InGrid)
{
	OwnerGrid = InGrid;
}

void UInventoryItemWidget::SetTooltipWidget(UInventoryTooltipWidget* InTooltip)
{
	TooltipWidget = InTooltip;
}

void UInventoryItemWidget::Refresh()
{
	//UE_LOG(LogTemp, Warning, TEXT("UInventoryItemWidget::Refresh()"));
	const FInventoryItemInstance* Item = GetItem();
	if (!Item)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	SetVisibility(ESlateVisibility::Visible);

	const FInventoryItemDefinition* Def = Inventory->GetItemDefinition(Item->ItemID);

	// 图标：旋转时优先使用预旋转纹理，ImageSize 使用 GetItemSize 获取旋转后的尺寸
	if (ItemIcon)
	{
		UTexture2D* IconTex = nullptr;
		if (Def && Def->Icon)
		{
			IconTex = Item->bRotated && Def->IconRotated ? Def->IconRotated : Def->Icon;
		}
		else if (Inventory.IsValid() && Inventory->DefaultItemRowID.IsValid())
		{
			if (const FInventoryItemDefinition* DefaultDef = Inventory->GetItemDefinition(Inventory->DefaultItemRowID))
			{
				IconTex = DefaultDef->Icon;
			}
		}

		if (IconTex)
		{
			const FIntPoint ItemSize = Inventory->GetItemSize(*Item);
			FSlateBrush Brush;
			Brush.SetResourceObject(IconTex);
			Brush.ImageSize = FVector2D(ItemSize.X * CellSize, ItemSize.Y * CellSize);
			ItemIcon->SetBrush(Brush);
			ItemIcon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// 堆叠数量：大于 1 时显示
	if (StackCount)
	{
		if (Def && Def->bStackable && Item->Quantity > 1)
		{
			StackCount->SetText(FText::AsNumber(Item->Quantity));
			StackCount->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			StackCount->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

const FInventoryItemInstance* UInventoryItemWidget::GetItem() const
{
	if (!Inventory.IsValid() || !ItemInstanceID.IsValid())
	{
		return nullptr;
	}
	return Inventory->FindItem(ItemInstanceID);
}

FReply UInventoryItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (OwnerGrid.IsValid() && ItemInstanceID.IsValid())
		{
			OwnerGrid->OnItemContextMenu(ItemInstanceID, InMouseEvent.GetScreenSpacePosition());
		}
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void UInventoryItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	const FInventoryItemInstance* Item = GetItem();
	if (!Item)
	{
		return;
	}

	UInventoryDragDropOperation* DragOp = NewObject<UInventoryDragDropOperation>();
	DragOp->ItemID = Item->InstanceID;
	DragOp->bRotated = Item->bRotated;
	DragOp->SourceInventory = Inventory;

	const FVector2D LocalMouse = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	const FIntPoint ItemSize = Inventory->GetItemSize(*Item);

	const FInventoryItemDefinition* Def = Inventory->GetItemDefinition(Item->ItemID);

	// 归一化抓取点：使用 GetItemSize 获取旋转感知的尺寸，与控件实际几何一致
	{
		const float W = ItemSize.X * CellSize;
		const float H = ItemSize.Y * CellSize;
		DragOp->NormalizedGrabPoint = FVector2D(
			FMath::Clamp(LocalMouse.X / W, 0.0f, 1.0f),
			FMath::Clamp(LocalMouse.Y / H, 0.0f, 1.0f)
		);
	}

	UImage* DragIcon = NewObject<UImage>(this);
	if (Def && Def->Icon)
	{
		const FVector2D DragIconSize(ItemSize.X * CellSize, ItemSize.Y * CellSize);
		UTexture2D* DragTex = (Item->bRotated && Def->IconRotated) ? Def->IconRotated : Def->Icon;

		FSlateBrush Brush;
		Brush.SetResourceObject(DragTex);
		Brush.ImageSize = DragIconSize;
		DragIcon->SetBrush(Brush);
	}

	DragOp->DefaultDragVisual = DragIcon;
	DragOp->DragVisualOffset = LocalMouse;
	DragOp->Pivot = EDragPivot::MouseDown;

	OutOperation = DragOp;
}

void UInventoryItemWidget::StartTooltipTimer(const FVector2D& MouseScreenPosition)
{
	// 同一时刻只允许一个 pending 定时器：先清除再设新
	if (TooltipTimerHandle.IsValid())
		GetWorld()->GetTimerManager().ClearTimer(TooltipTimerHandle);
	
	//UE_LOG(LogTemp, Warning, TEXT("UInventoryItemWidget::StartTooltipTimer"));
	TWeakObjectPtr<UInventoryTooltipWidget> WeakTooltip = TooltipWidget;
	TWeakObjectPtr<UInventoryComponent> WeakInventory = Inventory;
	const FGuid CachedItemID = ItemInstanceID;

	GetWorld()->GetTimerManager().SetTimer(TooltipTimerHandle, [WeakTooltip, WeakInventory, CachedItemID, MouseScreenPosition]()
	{
		if (WeakTooltip.IsValid() && WeakInventory.IsValid() && CachedItemID.IsValid())
		{
			WeakTooltip->SetItem(CachedItemID, WeakInventory.Get(), MouseScreenPosition);
		}
	}, TooltipDelaySeconds, false);
}

void UInventoryItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	PrevMouseScreenPosition = InMouseEvent.GetScreenSpacePosition();
	// 清除悬挂的旧定时器，启动新计时
	//UE_LOG(LogTemp, Warning, TEXT("NativeOnMouseEnter"));
	
	StartTooltipTimer(InMouseEvent.GetScreenSpacePosition());
}

FReply UInventoryItemWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	
	
	
	// 鼠标移动：立即隐藏工具提示并重新开始计时
	if (!PrevMouseScreenPosition.Equals(InMouseEvent.GetScreenSpacePosition(),0.1f)&& TooltipWidget.IsValid())
	{
		TooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
		StartTooltipTimer(InMouseEvent.GetScreenSpacePosition());
	
	}

	
	PrevMouseScreenPosition = InMouseEvent.GetScreenSpacePosition();
	
	return FReply::Unhandled();
}

void UInventoryItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	// 鼠标离开：取消悬挂的定时器并隐藏工具提示
	if (TooltipTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(TooltipTimerHandle);
	}

	if (TooltipWidget.IsValid())
	{
		TooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	Super::NativeOnMouseLeave(InMouseEvent);
}
