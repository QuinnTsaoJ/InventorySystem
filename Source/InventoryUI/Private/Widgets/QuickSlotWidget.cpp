// 快捷栏槽位控件实现
// InventorySystem Plugin

#include "Widgets/QuickSlotWidget.h"
#include "DragDrop/InventoryDragDropOperation.h"
#include "InventoryComponent.h"
#include "FInventoryItemInstance.h"
#include "FInventoryItemDefinition.h"
#include "FQuickSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UQuickSlotWidget::InitializeSlot(int32 InSlotIndex, UInventoryComponent* InInventory)
{
	SlotIndex = InSlotIndex;
	Inventory = InInventory;

	// 设置快捷键显示（1-9）
	if (HotkeyText)
	{
		HotkeyText->SetText(FText::AsNumber(InSlotIndex + 1));
	}

	Refresh();
}

void UQuickSlotWidget::Refresh()
{
	// 始终显示槽位框架，空槽位仅隐藏图标和数量
	SetVisibility(ESlateVisibility::Visible);

	if (!Inventory.IsValid())
	{
		return;
	}

	const TArray<FQuickSlot>& QuickSlots = Inventory->QuickSlots;
	if (SlotIndex < 0 || SlotIndex >= QuickSlots.Num())
	{
		if (ItemIcon) ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		if (StackCount) StackCount->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	const FGuid& ItemID = QuickSlots[SlotIndex].ItemInstanceID;
	if (!ItemID.IsValid())
	{
		if (ItemIcon) ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		if (StackCount) StackCount->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	const FInventoryItemInstance* Item = Inventory->FindItem(ItemID);
	if (!Item)
	{
		if (ItemIcon) ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		if (StackCount) StackCount->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	const FInventoryItemDefinition* Def = Inventory->GetItemDefinition(Item->ItemID);

	if (ItemIcon)
	{
		if (Def && Def->Icon)
		{
			ItemIcon->SetBrushFromTexture(Def->Icon);
			ItemIcon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		}
	}

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

FReply UQuickSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	return FReply::Unhandled();
}

void UQuickSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	if (!Inventory.IsValid())
	{
		return;
	}

	const TArray<FQuickSlot>& QuickSlots = Inventory->QuickSlots;
	if (SlotIndex < 0 || SlotIndex >= QuickSlots.Num())
	{
		return;
	}

	const FGuid& ItemID = QuickSlots[SlotIndex].ItemInstanceID;
	if (!ItemID.IsValid())
	{
		return;
	}

	UInventoryDragDropOperation* DragOp = NewObject<UInventoryDragDropOperation>();
	DragOp->ItemID = ItemID;
	DragOp->SourceInventory = Inventory;

	DragOp->DefaultDragVisual = this;
	DragOp->Pivot = EDragPivot::TopLeft;

	OutOperation = DragOp;
}

bool UQuickSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation){
	if (!Inventory.IsValid())
	{
		return false;
	}
	
	UInventoryDragDropOperation* DragOp = Cast<UInventoryDragDropOperation>(InOperation);
	if (!DragOp)
	{
		return false;
	}
	Inventory->SetQuickSlot(SlotIndex, DragOp->ItemID);
	Refresh();
	
	
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
