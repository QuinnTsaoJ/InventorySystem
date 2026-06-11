// 单个快捷栏槽位控件
// InventorySystem Plugin

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuickSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UInventoryComponent;

UCLASS()
class INVENTORYUI_API UQuickSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 初始化槽位 */
	void InitializeSlot(int32 InSlotIndex, UInventoryComponent* InInventory);

	/** 根据快捷栏当前绑定刷新显示 */
	void Refresh();

	/** 槽位索引 */
	int32 GetSlotIndex() const { return SlotIndex; }

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HotkeyText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StackCount;

	/** 槽位索引 */
	UPROPERTY()
	int32 SlotIndex = 0;

	/** 所属背包组件 */
	UPROPERTY()
	TWeakObjectPtr<UInventoryComponent> Inventory;
};
