# 按行名批量加入/移除物品 设计文档

## 背景

InventorySystem 插件已有 `AddItem(FInventoryItemInstance&)`（全有或全无语义）和 `RemoveItemsByID(FName, int32)`（返回 FInventoryOperationResult，但不含实际移除数量）。现需补充两个面向"行名 + 数量"的批量操作 API，语义为"尽量加入/移除，返回实际处理数量"。

## 需求

### 功能一：加入物品

- 外部传入物品数据表行名 + 数量，函数内部自动分组、堆叠并加入背包。
- 若数量超出背包空间或重量上限，只加入可容纳的部分。
- 返回实际加入的物品数量。

### 功能二：移除物品

- 现有 `RemoveItemsByID` 已实现按行名移除指定数量的逻辑，但返回值不含实际移除数量。
- 增强该函数：在返回的 `FInventoryOperationResult` 中填充实际移除数量，不新增函数、不改签名。

## 设计

### 1. 数据结构变更：FInventoryOperationResult 新增 Amount 字段

```cpp
UPROPERTY(BlueprintReadOnly)
int32 Amount = 0;  // 本次操作实际处理的物品数量
```

- 默认 0，由新函数/增强函数填充。
- 现有其他函数（AddItem / RemoveItem / MoveItem 等）暂不改动返回值，避免扩大影响面。

### 2. 功能一：新增 AddItemByID

```cpp
/**
 * 按物品数据表行名加入物品，自动分组堆叠。
 * 空间或重量不足时只加入可容纳的部分。
 * @param ItemRowID  物品数据表行名
 * @param Quantity   期望加入的数量（默认 1）
 * @return Result.Amount = 实际加入数量；全放不下时 ResultCode = NoSpace
 */
UFUNCTION(BlueprintCallable, Category = "Inventory")
FInventoryOperationResult AddItemByID(FName ItemRowID, int32 Quantity = 1);
```

#### 核心逻辑

1. 查物品定义 `GetItemDefinition(ItemRowID)`，无效 → `Amount=0, ResultCode=InvalidItem`。
2. **重量预算**：
   - 若 `Def->Weight > 0`：`MaxByWeight = floor((MaxWeight - CurrentWeight) / Weight)`
   - 若 `Def->Weight == 0`：`MaxByWeight = Quantity`（不限制）
   - `Quantity = min(Quantity, MaxByWeight)`；若 ≤ 0 → `Amount=0, ResultCode=Overweight`。
3. **可堆叠且无实例数据**（`bStackable && !bUseInstanceData`）：
   - 遍历已有同类型堆叠，填满直到 `MaxStackSize`，累计已加入数，逐个广播 `OnItemUpdated`，记录 AffectedItems。
   - 扣减剩余待加入数量。
4. **剩余部分占新格**：
   - 可堆叠：按 `MaxStackSize` 分组，每组构造临时实例，`FindAvailablePosition` 找位置占新格，放不下则停止。
   - 不可堆叠（或带实例数据）：逐个占 1 格，放不下则停止。
   - 每个新堆叠广播 `OnItemAdded`，记录 AffectedItems。
5. `RebuildOccupancyMap()` + `RecalculateWeight()`。
6. 广播 `OnWeightChanged` + `OnInventoryChanged`。
7. `Amount = 实际加入数`；`bSuccess = (Amount > 0)`；全放不下（Amount==0）时 `ResultCode = NoSpace`。

#### 语义说明

- 部分 success（加入 ≥1 个但 < 期望数量）也算 `bSuccess = true`。
- 只有完全一个都加不进去才算失败（Amount==0）。
- 与现有 `AddItem` 的"全有或全无"语义不同，独立实现，不大改现有 `AddItem`。

### 3. 功能二：增强 RemoveItemsByID

现有函数签名不变：

```cpp
UFUNCTION(BlueprintCallable, Category = "Inventory")
FInventoryOperationResult RemoveItemsByID(FName ItemRowID, int32 Count = 0);
```

仅改动内部实现：在移除/扣减过程中累计实际移除数量，最终填入 `Result.Amount`。

- `Count <= 0`：移除该类型全部，`Amount = 实际移除总数`。
- `Count > 0`：移除至多 Count 个，`Amount = min(Count, 背包内该类型总数)`。

## 涉及文件

| 文件 | 改动 |
|------|------|
| `Public/InventoryComponent.h` | FInventoryOperationResult 新增 Amount 字段；声明 AddItemByID |
| `Private/InventoryComponent.cpp` | 实现 AddItemByID；增强 RemoveItemsByID 填充 Amount |
| `DOCUMENTATION.md` | API 表格补充 AddItemByID；委托/错误码说明补充 Amount |
| `README.md` | 功能列表补充 |

## 验证

- 编译通过（UBT 构建 InventorySystem 模块）。
- 蓝图可调用 AddItemByID 并读取 Result.Amount。
- RemoveItemsByID 的 Result.Amount 正确反映实际移除数量。
