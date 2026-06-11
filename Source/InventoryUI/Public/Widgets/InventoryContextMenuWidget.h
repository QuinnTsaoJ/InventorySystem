// 物品右键菜单控件 —— C++ 提供 Tag 驱动的基础架构，Blueprint 负责视觉效果
// InventorySystem Plugin

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "Components/VerticalBox.h"
#include "InventoryContextMenuWidget.generated.h"

class UPanelWidget;
class UInventoryComponent;

UCLASS()
class INVENTORYUI_API UInventoryContextMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 初始化菜单：接收可用操作标签、物品标识、背包引用和屏幕位置 */
	void InitializeMenu(const TArray<FGameplayTag>& Actions, const FGuid& InItemID, UInventoryComponent* InInventory, const FVector2D& ScreenPosition);

	/** Blueprint 调用：选中某个操作标签，C++ 转发到 InventoryComponent::ExecuteAction */
	UFUNCTION(BlueprintCallable, Category = "Inventory|ContextMenu")
	void SelectAction(FGameplayTag ActionTag);

	/** Blueprint 调用：关闭本菜单 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|ContextMenu")
	void CloseMenu();
	
protected:
	/** Blueprint 实现：根据给定标签数组创建视觉菜单项，每个项绑定 SelectAction */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|ContextMenu")
	void OnBuildMenu(const TArray<FGameplayTag>& Actions, const FGuid& ActionItemID);

	UPROPERTY(BlueprintReadOnly,meta = (BindWidget))
	TObjectPtr<UVerticalBox> EntryContainer;
	
	

private:
	UPROPERTY()
	FGuid ItemID;

	UPROPERTY()
	TWeakObjectPtr<UInventoryComponent> Inventory;
};
