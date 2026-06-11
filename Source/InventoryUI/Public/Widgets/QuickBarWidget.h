// 快捷栏容器控件 —— 管理一组 QuickSlotWidget
// InventorySystem Plugin

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuickBarWidget.generated.h"

class UHorizontalBox;
class UQuickSlotWidget;
class UInventoryComponent;

UCLASS()
class INVENTORYUI_API UQuickBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 绑定背包组件并刷新所有槽位 */
	void InitializeQuickBar(UInventoryComponent* InInventory);

	/** 刷新所有槽位 */
	void RefreshAllSlots();

	/** QuickSlot 蓝图类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSubclassOf<UQuickSlotWidget> QuickSlotClass;

	/** 快捷栏槽位数量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 SlotCount = 6;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> SlotPanel;

	UPROPERTY()
	TWeakObjectPtr<UInventoryComponent> Inventory;

	UPROPERTY()
	TArray<UQuickSlotWidget*> QuickSlots;

	bool bQuickSlotBound = false;

	UFUNCTION()
	void OnQuickSlotChanged();

	UFUNCTION()
	void OnInventoryCleared();

	UFUNCTION()
	void OnInventoryItemRemoved(const FGuid& InstanceID);

	void ClearQuickSlotForItem(const FGuid& InstanceID);
	
	void ClearAllSlots();
};
