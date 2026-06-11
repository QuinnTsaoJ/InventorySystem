// 背包交互组件 —— 挂载在 PlayerController 上，管理 UI 生命周期、输入、会话
//
// 架构位置：
//   PlayerController
//       └── UInventoryInteractionComponent
//               ├── InventoryPanelWidget（创建一次，显隐切换）
//               ├── PlayerInventory（通过 IInventoryProviderInterface 解析）
//               ├── ExternalInventory（外部容器交互）
//               └── Input（Tab / ESC / R / 1-9）
//
// InventoryUI Module

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Components/ActorComponent.h"
#include "Widgets/InventoryPanelWidget.h"
#include "InventoryInteractionComponent.generated.h"

class UInventoryPanelWidget;
class UQuickBarWidget;
class UInventoryComponent;

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class INVENTORYUI_API UInventoryInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryInteractionComponent();
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	//---------------------------- 公共 API ---------------------------

	/** 打开自己的背包（Tab 键），仅玩家背包 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OpenPlayerInventory();

	/** 打开外部容器（战利品箱等），同时显示玩家背包和目标容器 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OpenExternalInventory(UInventoryComponent* InExternalInventory);

	/** 关闭所有背包面板 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CloseInventory();

	/** 切换背包显隐 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ToggleInventory();

	/** 背包是否打开 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool IsInventoryOpen() const { return InventoryWidget->IsInViewport(); }

	/** 当前玩家背包组件 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UInventoryComponent* GetPlayerInventory() const { return PlayerInventory.Get(); }

	/** 当前打开的外部容器 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UInventoryComponent* GetExternalInventory() const { return ExternalInventory.Get(); }

	//---------------------------- 配置 ---------------------------

	/** InventoryPanelWidget 蓝图类 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|UI")
	TSubclassOf<UInventoryPanelWidget> InventoryWidgetClass;

	/** QuickBarWidget 蓝图类（独立于背包面板，始终可见） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|UI")
	TSubclassOf<UQuickBarWidget> QuickBarWidgetClass;

	//---------------------------- Blueprint 扩展点 ---------------------------

	/** 背包打开时触发 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnInventoryOpened();

	/** 背包关闭时触发 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnInventoryClosed();

	/** 外部容器打开时触发 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnExternalInventoryOpened(UInventoryComponent* ExternalInv);

	/** 快捷栏使用物品时触发 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnQuickSlotUsed(int32 SlotIndex);
	

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="EnhancedInput|Action", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> IA_ToggleInventory;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="EnhancedInput|Action", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> IA_CloseInventory;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="EnhancedInput|Action", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> IA_RotateItem;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="EnhancedInput|Action", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> IA_QuickSlot1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="EnhancedInput|Action", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> IA_QuickSlot2;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="EnhancedInput|Action", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> IA_QuickSlot3;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="EnhancedInput|Action", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> IA_QuickSlot4;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="EnhancedInput|Action", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> IA_QuickSlot5;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="EnhancedInput|Action", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> IA_QuickSlot6;
	
private:
	//---------------------------- 内部状态 ---------------------------

	UPROPERTY()
	TObjectPtr<UInventoryPanelWidget> InventoryWidget;

	UPROPERTY()
	TObjectPtr<UQuickBarWidget> QuickBarWidget;

	UPROPERTY()
	TWeakObjectPtr<UInventoryComponent> PlayerInventory;

	UPROPERTY()
	TWeakObjectPtr<UInventoryComponent> ExternalInventory;

	//---------------------------- 内部方法 ---------------------------

	/** 统一会话初始化 */
	void OpenInventorySession(UInventoryComponent* InPlayerInventory, UInventoryComponent* InExternalInventory);

	/** 通过 IInventoryProviderInterface 从 Pawn 解析背包 */
	UInventoryComponent* ResolvePlayerInventory() const;

	/** 设置输入模式（UI + 游戏，或纯游戏） */
	void SetInputModeForInventory(bool bOpen);

	/** 绑定输入 */
	void SetupInputBindings();
	void RemoveInputBindings();

	// 输入回调
	void OnInputToggleInventory();
	void OnInputCloseInventory();
	void OnInputRotateItem();
	void OnInputQuickSlot1() { HandleQuickSlotInput(0); }
	void OnInputQuickSlot2() { HandleQuickSlotInput(1); }
	void OnInputQuickSlot3() { HandleQuickSlotInput(2); }
	void OnInputQuickSlot4() { HandleQuickSlotInput(3); }
	void OnInputQuickSlot5() { HandleQuickSlotInput(4); }
	void OnInputQuickSlot6() { HandleQuickSlotInput(5); }
	void HandleQuickSlotInput(int32 SlotIndex);

	UPROPERTY()
	TObjectPtr<UInputComponent> CachedInputComponent;
};
