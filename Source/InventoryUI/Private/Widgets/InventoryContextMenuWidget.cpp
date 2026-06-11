// 物品右键菜单控件实现
// InventorySystem Plugin

#include "Widgets/InventoryContextMenuWidget.h"
#include "InventoryComponent.h"
#include "Components/PanelWidget.h"
#include "Components/CanvasPanelSlot.h"

void UInventoryContextMenuWidget::InitializeMenu(const TArray<FGameplayTag>& Actions, const FGuid& InItemID, UInventoryComponent* InInventory, const FVector2D& ScreenPosition)
{
	ItemID = InItemID;
	Inventory = InInventory;

	// 通过 AddToViewport 添加的控件 Slot 为 CanvasPanelSlot，直接设置位置即可
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		const FGeometry CanvasGeometry = CanvasSlot->Parent->GetCachedGeometry();
		const FVector2D LocalMouse = CanvasGeometry.AbsoluteToLocal(ScreenPosition);
		CanvasSlot->SetPosition(LocalMouse);
	}
	SetVisibility(ESlateVisibility::Visible);
	SetFocus();
	// 通知 Blueprint 构建视觉菜单项
	OnBuildMenu(Actions, InItemID);
}

void UInventoryContextMenuWidget::SelectAction(FGameplayTag ActionTag)
{
	if (!Inventory.IsValid() || !ItemID.IsValid())
	{
		return;
	}

	Inventory->ExecuteAction(ActionTag, ItemID);
	CloseMenu();
}

void UInventoryContextMenuWidget::CloseMenu(){
	EntryContainer->ClearChildren();
	SetVisibility(ESlateVisibility::Collapsed);
}
