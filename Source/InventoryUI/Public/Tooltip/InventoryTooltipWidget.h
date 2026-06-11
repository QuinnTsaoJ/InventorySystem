// 物品悬浮提示控件 —— 由 Blueprint 扩展 RuntimeData 显示
// InventorySystem Plugin

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryTooltipWidget.generated.h"

class UImage;
class UTextBlock;
class UInventoryComponent;

UCLASS()
class INVENTORYUI_API UInventoryTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 设置要显示的物品，从 Runtime 拉取数据，并将右上角对齐到屏幕坐标 */
	void SetItem(const FGuid& ItemID, UInventoryComponent* InInventory, const FVector2D& MouseScreenPosition);

protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemWeight;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemDescription;

	/** Blueprint 扩展点：用于显示 RuntimeData（新鲜度、温度等） */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnRuntimeDataDisplay(const FGuid& ItemID);

private:
	UPROPERTY()
	TWeakObjectPtr<UInventoryComponent> Inventory;

	UPROPERTY()
	FGuid CurrentItemID;
};
