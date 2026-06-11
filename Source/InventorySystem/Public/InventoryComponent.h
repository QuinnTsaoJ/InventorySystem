// 背包系统核心组件 —— 管理物品数据、格子占用、重量计算
// InventorySystem Plugin

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FInventoryItemDefinition.h"
#include "FInventoryItemInstance.h"
#include "FQuickSlot.h"
#include "FInventorySaveData.h"
#include "InventoryTags.h"
#include "ErrorTypeEnum.h"
#include "InventoryComponent.generated.h"


USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FInventoryOperationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly)
	EInventoryResult ResultCode = EInventoryResult::Success;

	UPROPERTY(BlueprintReadOnly)
	FText Message;

	UPROPERTY(BlueprintReadOnly)
	TArray<FGuid> AffectedItems;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemAdded, const FInventoryItemInstance&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemRemoved, const FGuid&, InstanceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemMoved, const FGuid&, InstanceID, FIntPoint, NewPosition, bool, bRotated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUpdated, const FGuid&, InstanceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryResized);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeightChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuickSlotChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUseRequested, const FGuid&, InstanceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDropped, const FGuid&, InstanceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryCleared);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEM_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//---------------------------- 委托 ---------------------------

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemRemoved OnItemRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemMoved OnItemMoved;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemUpdated OnItemUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryResized OnInventoryResized;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnWeightChanged OnWeightChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnQuickSlotChanged OnQuickSlotChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemUseRequested OnItemUseRequested;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemDropped OnItemDropped;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryCleared OnInventoryCleared;

	//---------------------------- 查询类 API ---------------------------

	const FInventoryItemDefinition* GetItemDefinition(FName ItemID) const;
	FInventoryItemInstance* FindItem(const FGuid& InstanceID);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const TArray<FInventoryItemInstance>& GetItems() const;

	/** 查找背包中所有指定 ItemRowID 的物品，返回 InstanceID 数组 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FGuid> FindItemsByID(FName ItemRowID) const;

	/** 背包中某类物品的总数量（跨所有堆叠） */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemCount(FName ItemRowID) const;

	/** 背包中是否至少有一个指定类型的物品 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasItem(FName ItemRowID) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	float GetMaxWeight() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool CanCarryWeight(float AdditionalWeight) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool IsCellOccupied(FIntPoint Cell) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool CanPlaceItem(const FInventoryItemInstance& Item, FIntPoint Position, int32 IgnoreIndex = -1) const;

	/** 获取物品占格尺寸，内部处理 bRotated 旋转 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FIntPoint GetItemSize(const FInventoryItemInstance& Item) const;

	//---------------------------- 修改类 API ---------------------------

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOperationResult AddItem(const FInventoryItemInstance& Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOperationResult RemoveItem(const FGuid& InstanceID);

	/** 丢弃物品：从背包中移除并触发 OnItemDropped 委托，供外部实现具体的丢弃到世界等行为 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOperationResult DropItem(const FGuid& InstanceID);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOperationResult MoveItem(const FGuid& InstanceID, FIntPoint NewPosition, bool bRotated = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOperationResult RotateItem(const FGuid& InstanceID);

	/** 将 SourceItemID 的堆叠数量合并到 TargetItemID，源物品随后被移除 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOperationResult MergeStack(const FGuid& SourceItemID, const FGuid& TargetItemID);

	/** 跨背包转移：从当前背包移除物品，添加到目标背包的指定位置 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOperationResult TransferItem(const FGuid& InstanceID, UInventoryComponent* TargetInventory, FIntPoint TargetPosition, bool bRotated = false);

	/** @param SplitAmount 拆分到新堆叠的数量 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOperationResult SplitStack(const FGuid& InstanceID, int32 SplitAmount);

	/** 重新整理：大物品优先，从左到右、从上到下重新摆放 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOperationResult AutoArrange();

	/** @param NewSize 新的背包格子尺寸 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOperationResult ResizeInventory(FIntPoint NewSize);

	/** 清空背包内所有物品 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOperationResult RemoveAll();

	/** 移除指定数量的某类物品（从任意堆叠中扣除），Count <= 0 表示全部移除 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOperationResult RemoveItemsByID(FName ItemRowID, int32 Count = 0);

	//---------------------------- 存档 API ---------------------------

	/** 将当前背包状态导出为存档数据 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
	FInventorySaveData SaveToData() const;

	/** 从存档数据恢复背包状态，会清空当前所有物品并广播事件 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
	void LoadFromData(const FInventorySaveData& Data);

	//---------------------------- 快捷栏 API ---------------------------

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool SetQuickSlot(int32 SlotIndex, const FGuid& ItemID);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool ClearQuickSlot(int32 SlotIndex);

	/** 转发到 UseItem */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseQuickSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseItem(const FInventoryItemInstance& Item);

	//---------------------------- 右键菜单 API ---------------------------

	/** 查询某物品的可用操作标签列表（从物品定义的 EnabledActions 读取） */
	UFUNCTION(BlueprintCallable, Category = "Inventory|ContextMenu")
	TArray<FGameplayTag> GetAvailableActions(const FGuid& InstanceID) const;

	/** 执行操作标签对应的逻辑。内置标签走 C++ 分发，自定义标签转发给 Blueprint 事件 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|ContextMenu")
	bool ExecuteAction(FGameplayTag ActionTag, const FGuid& InstanceID);

	/** 自定义操作标签处理（Blueprint 扩展点）。仅当 ExecuteAction 遇到未知标签时调用 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|ContextMenu")
	void OnCustomAction(FGameplayTag ActionTag, const FGuid& InstanceID);

	//---------------------------- 配置 ---------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Data", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataTable> ItemDefinitionTable;

	/** 默认物品 RowID，当物品定义的 Icon 为空时用此定义的图标兜底显示 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Data", meta = (AllowPrivateAccess = "true"))
	FName DefaultItemRowID = "00001";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Config", meta = (AllowPrivateAccess = "true"))
	FIntPoint GridSize = FIntPoint(10, 6);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Config", meta = (AllowPrivateAccess = "true"))
	float MaxWeight = 100.f;

	//---------------------------- 运行时状态 ---------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|State", meta = (AllowPrivateAccess = "true"))
	TArray<FInventoryItemInstance> Items;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|State", meta = (AllowPrivateAccess = "true"))
	TArray<FGuid> OccupancyMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|State", meta = (AllowPrivateAccess = "true"))
	float CurrentWeight = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|State", meta = (AllowPrivateAccess = "true"))
	TArray<FQuickSlot> QuickSlots;

	// 物品定义缓存，避免重复查表。GetItemDefinition 为 const 方法，缓存用 mutable
	mutable TMap<FName, const FInventoryItemDefinition*> DefinitionCache;

	//---------------------------- 内部辅助函数 ---------------------------

	/** 从左到右、从上到下搜索第一个可用位置 */
	bool FindAvailablePosition(const FInventoryItemInstance& Item, FIntPoint& OutPosition) const;

	/** 根据 Items 数组全量重建 OccupancyMap。FGuid() 表示空格，有效 GUID 表示占用物品的 InstanceID */
	void RebuildOccupancyMap();

	/** 遍历 Items 重新计算当前总重量 */
	void RecalculateWeight();

};
