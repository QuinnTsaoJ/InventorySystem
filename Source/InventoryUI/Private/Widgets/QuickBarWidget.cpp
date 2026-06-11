// 快捷栏容器控件实现
// InventorySystem Plugin

#include "Widgets/QuickBarWidget.h"
#include "Widgets/QuickSlotWidget.h"
#include "InventoryComponent.h"
#include "Components/HorizontalBox.h"

void UQuickBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UQuickBarWidget::NativeDestruct()
{
	if (bQuickSlotBound && Inventory.IsValid())
	{
		Inventory->OnQuickSlotChanged.RemoveDynamic(this, &UQuickBarWidget::OnQuickSlotChanged);
		Inventory->OnInventoryCleared.RemoveDynamic(this, &UQuickBarWidget::OnInventoryCleared);
		Inventory->OnItemRemoved.RemoveDynamic(this, &UQuickBarWidget::OnInventoryItemRemoved);
		bQuickSlotBound = false;
	}
	ClearAllSlots();
	Super::NativeDestruct();
}

void UQuickBarWidget::InitializeQuickBar(UInventoryComponent* InInventory)
{
	Inventory = InInventory;
	if (!Inventory.IsValid() || !QuickSlotClass || !SlotPanel)
	{
		return;
	}

	// 绑定委托（避免重复绑定）
	if (!bQuickSlotBound)
	{
		Inventory->OnQuickSlotChanged.AddDynamic(this, &UQuickBarWidget::OnQuickSlotChanged);
		Inventory->OnInventoryCleared.AddDynamic(this, &UQuickBarWidget::OnInventoryCleared);
		Inventory->OnItemRemoved.AddDynamic(this, &UQuickBarWidget::OnInventoryItemRemoved);
		bQuickSlotBound = true;
	}

	// 创建固定数量的槽位
	ClearAllSlots();
	QuickSlots.SetNum(SlotCount);

	for (int32 i = 0; i < SlotCount; ++i)
	{
		if (UQuickSlotWidget* QuickSlotPtr= CreateWidget<UQuickSlotWidget>(GetOwningPlayer(), QuickSlotClass))
		{
			QuickSlotPtr->InitializeSlot(i, Inventory.Get());
			SlotPanel->AddChildToHorizontalBox(QuickSlotPtr);
			QuickSlots[i] = QuickSlotPtr;
		}
	}
}

void UQuickBarWidget::RefreshAllSlots()
{
	for (UQuickSlotWidget* QuickSlotPtr: QuickSlots)
	{
		if (QuickSlotPtr)
		{
			QuickSlotPtr->Refresh();
		}
	}
}

void UQuickBarWidget::OnQuickSlotChanged()
{
	RefreshAllSlots();
}

void UQuickBarWidget::OnInventoryCleared()
{
	RefreshAllSlots();
}

void UQuickBarWidget::OnInventoryItemRemoved(const FGuid& InstanceID)
{
	ClearQuickSlotForItem(InstanceID);
}

void UQuickBarWidget::ClearQuickSlotForItem(const FGuid& InstanceID)
{
	if (!Inventory.IsValid())
	{
		return;
	}

	for (int32 i = 0; i < Inventory->QuickSlots.Num(); ++i)
	{
		if (Inventory->QuickSlots[i].ItemInstanceID == InstanceID)
		{
			Inventory->ClearQuickSlot(i);
		}
	}
}

void UQuickBarWidget::ClearAllSlots()
{
	for (UQuickSlotWidget* QuickSlotPtr: QuickSlots)
	{
		if (QuickSlotPtr)
		{
			QuickSlotPtr->RemoveFromParent();
		}
	}
	QuickSlots.Empty();
}
