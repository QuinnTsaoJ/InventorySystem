// 背包根面板 —— 组合 Grid + QuickBar + Tooltip，负责打开/关闭
// InventorySystem Plugin

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryPanelWidget.generated.h"

class UInventoryGridWidget;
class UInventoryContextMenuWidget;
class UQuickBarWidget;
class UInventoryTooltipWidget;
class UInventoryComponent;
class UHorizontalBox;

UCLASS()
class INVENTORYUI_API UInventoryPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	//---------------------------- 初始化 ---------------------------

	/** 绑定单个背包组件（玩家背包），初始化所有子控件 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeInventory(UInventoryComponent* InInventory);

	/** 同时初始化玩家背包和外部容器，自动创建第二个网格控件（双网格模式） */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeDualInventory(UInventoryComponent* InPlayerInventory, UInventoryComponent* InExternalInventory);

	/** 打开/关闭背包面板 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ToggleInventory();

	/** 关闭背包面板 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CloseInventory();

	/** 获取当前绑定的背包组件 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UInventoryComponent* GetInventory() const { return Inventory.Get(); }

	/** 获取当前打开的外部容器 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UInventoryComponent* GetExternalInventory() const { return ExternalInventory.Get(); }

	//---------------------------- Blueprint 扩展点 ---------------------------

	/** 双击物品时触发，由 Blueprint 实现具体行为（使用、装备等） */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnItemDoubleClicked(const FGuid& ItemID);

	/** 右键物品时触发：查询可用操作标签，创建菜单控件并添加到视口 */
	void OnItemContextRequested(UInventoryComponent* ItemInventory,const FGuid& ItemID, const FVector2D& ScreenPosition);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	//---------------------------- 子控件（Blueprint 绑定） ---------------------------

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryGridWidget> InventoryGrid;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryTooltipWidget> InventoryTooltipWidget;

	/** 网格容器（HorizontalBox），负责单/双网格的自动水平排列与居中 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> GridContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryContextMenuWidget> ContextMenu;
	
	/** 外部容器网格控件（双开时打开） */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryGridWidget> ExternalInventoryGrid;

	/** 拖拽过程中按下 R 键旋转 */
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	UPROPERTY()
	TWeakObjectPtr<UInventoryComponent> Inventory;

	UPROPERTY()
	TWeakObjectPtr<UInventoryComponent> ExternalInventory;
	

	UFUNCTION()
	void OnWeightChanged();

	bool bWeightBound = false;

	/** 清理动态创建的外部网格控件 */
	void ClearExternalGrid();
};
