// 背包根面板实现
// InventorySystem Plugin

#include "Widgets/InventoryPanelWidget.h"
#include "Widgets/InventoryGridWidget.h"
#include "Widgets/InventoryContextMenuWidget.h"
#include "Tooltip/InventoryTooltipWidget.h"
#include "DragDrop/InventoryDragDropOperation.h"
#include "InventoryComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"

void UInventoryPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 设为可获取焦点，确保 NativeOnKeyDown 能收到键盘事件
	SetIsFocusable(true);
}

void UInventoryPanelWidget::NativeDestruct()
{
	ClearExternalGrid();

	if (bWeightBound && Inventory.IsValid())
	{
		Inventory->OnWeightChanged.RemoveDynamic(this, &UInventoryPanelWidget::OnWeightChanged);
		bWeightBound = false;
	}
	Super::NativeDestruct();
}

void UInventoryPanelWidget::InitializeInventory(UInventoryComponent* InInventory)
{

	Inventory = InInventory;
	if (!Inventory.IsValid())
	{
		return;
	}

	// 初始化子控件 —— 先注入依赖引用，再 InitializeGrid，
	// 否则 RefreshAllItems → CreateItemWidget 时 TooltipWidget/ParentPanel 仍为空
	if (InventoryGrid)
	{
		InventoryGrid->SetParentPanel(this);
		InventoryGrid->SetTooltipWidget(InventoryTooltipWidget);
		InventoryGrid->InitializeGrid(Inventory.Get());
	}

	// 绑定重量变化委托（避免重复绑定）
	if (!bWeightBound)
	{
		Inventory->OnWeightChanged.AddDynamic(this, &UInventoryPanelWidget::OnWeightChanged);
		bWeightBound = true;
	}

	if (ExternalInventoryGrid)
		ExternalInventoryGrid->SetVisibility(ESlateVisibility::Collapsed);
	SetVisibility(ESlateVisibility::Visible);
}

void UInventoryPanelWidget::InitializeDualInventory(UInventoryComponent* InPlayerInventory, UInventoryComponent* InExternalInventory)
{
	if (!InPlayerInventory)
	{
		return;
	}

	if (InExternalInventory!=ExternalInventory) {
		// 清理上一次双网格模式残留的外部网格
		ClearExternalGrid();
	}

	// 先初始化玩家网格
	InitializeInventory(InPlayerInventory);

	if (!InExternalInventory || !GridContainer)
	{
		return;
	}

	ExternalInventory = InExternalInventory;

	if (ExternalInventoryGrid)
	{
		ExternalInventoryGrid->SetVisibility(ESlateVisibility::Visible);
		ExternalInventoryGrid->SetParentPanel(this);
		ExternalInventoryGrid->SetTooltipWidget(InventoryTooltipWidget);
		ExternalInventoryGrid->InitializeGrid(InExternalInventory);
	}
}

void UInventoryPanelWidget::ClearExternalGrid()
{
	if (ExternalInventoryGrid)
	{
		ExternalInventoryGrid->ClearInventory();
		ExternalInventoryGrid->SetVisibility(ESlateVisibility::Collapsed);
	}

}

void UInventoryPanelWidget::ToggleInventory()
{
	if (GetVisibility() == ESlateVisibility::Visible)
	{
		CloseInventory();
	}
	else
	{
		SetVisibility(ESlateVisibility::Visible);
	}
}

void UInventoryPanelWidget::OnItemContextRequested(UInventoryComponent* ItemInventory,const FGuid& ItemID, const FVector2D& ScreenPosition)
{
	if (!ItemInventory)
	{
		return;
	}

	const TArray<FGameplayTag> Actions = ItemInventory->GetAvailableActions(ItemID);
	if (Actions.IsEmpty())
	{
		return;
	}

	if (ContextMenu)
	{
		ContextMenu->InitializeMenu(Actions, ItemID, ItemInventory, ScreenPosition);
	}
}

void UInventoryPanelWidget::CloseInventory()
{
	SetVisibility(ESlateVisibility::Collapsed);

	if (InventoryTooltipWidget)
	{
		InventoryTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 恢复纯游戏输入模式，隐藏鼠标
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}

FReply UInventoryPanelWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::R)
	{
		// 拖拽期间按 R 键旋转物品
		if (UDragDropOperation* DragOp = UWidgetBlueprintLibrary::GetDragDroppingContent())
		{
			if (UInventoryDragDropOperation* InvDragOp = Cast<UInventoryDragDropOperation>(DragOp))
			{
				InvDragOp->RequestRotate();
				return FReply::Handled();
			}
		}
	}

	// 数字键 1-9 映射到快捷栏
	if (InKeyEvent.GetKey() == EKeys::Escape)    { CloseInventory(); return FReply::Handled(); }
	if (InKeyEvent.GetKey() == EKeys::Tab)    { ToggleInventory(); return FReply::Handled(); }

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UInventoryPanelWidget::OnWeightChanged()
{
	// 重量变化时可以由 Blueprint 子类重载 OnWeightChanged 事件来更新 UI
}
