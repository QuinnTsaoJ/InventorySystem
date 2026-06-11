// 物品悬浮提示控件实现
// InventorySystem Plugin

#include "Tooltip/InventoryTooltipWidget.h"
#include "InventoryComponent.h"
#include "FInventoryItemDefinition.h"
#include "FInventoryItemInstance.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"

void UInventoryTooltipWidget::SetItem(const FGuid& ItemID, UInventoryComponent* InInventory, const FVector2D& MouseScreenPosition)
{
	CurrentItemID = ItemID;
	Inventory = InInventory;

	if (!Inventory.IsValid() || !ItemID.IsValid())
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const FInventoryItemInstance* Item = Inventory->FindItem(ItemID);
	if (!Item)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const FInventoryItemDefinition* Def = Inventory->GetItemDefinition(Item->ItemID);
	if (!Def)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (ItemName)
	{
		ItemName->SetText(Def->Name);
	}

	if (ItemWeight)
	{
		const float TotalWeight = Def->Weight * Item->Quantity;
		ItemWeight->SetText(FText::AsNumber(TotalWeight));
	}

	if (ItemDescription)
	{
		ItemDescription->SetText(Def->Description);
	}

	// Blueprint 扩展点：显示 RuntimeData
	OnRuntimeDataDisplay(ItemID);

	SetVisibility(ESlateVisibility::Visible);

	// 定位：右上角对齐鼠标位置
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		if (CanvasSlot->Parent)
		{
			const FGeometry CanvasGeometry = CanvasSlot->Parent->GetCachedGeometry();
			const FVector2D LocalMouse = CanvasGeometry.AbsoluteToLocal(MouseScreenPosition);

			CanvasSlot->SetPosition(FVector2D(
				LocalMouse.X+10 ,
				LocalMouse.Y+10
			));
		}
	}
}
