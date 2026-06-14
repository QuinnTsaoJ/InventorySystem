# InventorySystem 插件文档

## 概述

InventorySystem 是一个 UE5 网格背包插件，提供物品数据管理、格子占用检测、拖拽交互、快捷栏、右键菜单和存档支持。插件采用 **Definition + Instance** 双层架构，物品模板通过 DataTable 定义，运行时生成带唯一 GUID 的实例。

插件负责**数据与 UI 基础设施**，不包含具体玩法逻辑。使用效果、合成、丢弃到世界等行为由游戏层通过委托和 GameplayTag 扩展实现。

---

## 模块结构

| 模块 | 类型 | 依赖 | 职责 |
|------|------|------|------|
| **InventorySystem** | Runtime | Core, GameplayTags, EnhancedInput | 数据结构、背包组件、物品操作 |
| **InventoryUI** | Runtime | InventorySystem, UMG, Slate, InputCore | UI 控件、拖拽操作、交互组件 |

---

## 架构

```
DataTable (FInventoryItemDefinition)
    │
    ├── UInventoryComponent  ── Actor 挂载（角色 / 容器）
    │       ├── Items: TArray<FInventoryItemInstance>
    │       ├── OccupancyMap: TArray<FGuid>
    │       ├── QuickSlots: TArray<FQuickSlot>
    │       └── Delegates: 10 个 BlueprintAssignable
    │
    ├── UInventoryInteractionComponent  ── PlayerController 挂载
    │       ├── 管理 UI 生命周期、输入绑定、会话
    │       └── 通过 IInventoryProviderInterface 解析背包
    │
    └── UI Widgets  ── UMG 控件树
            ├── UInventoryPanelWidget       根面板
            ├── UInventoryGridWidget        网格（物品图标、拖拽预览）
            ├── UInventoryItemWidget        单个物品
            ├── UInventoryTooltipWidget     悬浮提示
            ├── UInventoryContextMenuWidget 右键菜单
            ├── UQuickBarWidget             快捷栏容器
            └── UQuickSlotWidget            单个快捷槽位
```

---

## 数据结构

### FInventoryItemDefinition（物品定义，继承 FTableRowBase）

物品的静态数据模板，在 DataTable 中配置。每个物品对应一行。

| 字段 | 类型 | 说明 |
|------|------|------|
| Name | FText | 物品名称 |
| Description | FText | 物品描述 |
| Icon | TObjectPtr\<UTexture2D\> | 图标纹理 |
| IconRotated | TObjectPtr\<UTexture2D\> | 旋转后图标（可选） |
| Size | FIntPoint | 占格尺寸（宽 x 高） |
| Weight | float | 单个重量 |
| bCanRotate | bool | 是否可旋转 |
| bStackable | bool | 是否可堆叠 |
| MaxStackSize | int32 | 最大堆叠数量 |
| bUseInstanceData | bool | 是否需要实例数据（带实例数据的物品不可堆叠） |
| ItemTags | FGameplayTagContainer | 物品标签（类型、属性等） |
| EnabledActions | FGameplayTagContainer | 右键菜单可用操作 Tag |
| RuntimeDataType | TInstancedStruct\<FItemRuntimeData\> | 运行时数据类型模板 |

### FInventoryItemInstance（物品实例）

运行时的物品个体，每个实例拥有唯一 GUID。

| 字段 | 类型 | 说明 |
|------|------|------|
| ItemID | FName | 对应 DataTable 行名 |
| InstanceID | FGuid | 唯一实例 ID |
| Quantity | int32 | 数量 |
| Position | FIntPoint | 左上角格子坐标 |
| bRotated | bool | 是否旋转 |
| bStackable | bool | 是否可堆叠（冗余，方便运行时读取） |
| RuntimeData | FInstancedStruct | 运行时动态数据（对应定义中的 RuntimeDataType） |

### FQuickSlot（快捷栏槽位）

| 字段 | 类型 | 说明 |
|------|------|------|
| ItemInstanceID | FGuid | 绑定的物品实例 ID |

### FInventorySaveData（存档数据）

| 字段 | 类型 | 说明 |
|------|------|------|
| Items | TArray\<FInventoryItemInstance\> | 所有物品实例 |
| QuickSlots | TArray\<FQuickSlot\> | 快捷栏绑定 |
| GridSize | FIntPoint | 背包格子尺寸 |
| MaxWeight | float | 最大承重 |

### FInventoryOperationResult（操作结果）

| 字段 | 类型 | 说明 |
|------|------|------|
| bSuccess | bool | 操作是否成功 |
| ResultCode | EInventoryResult | 错误码枚举 |
| Message | FText | 错误描述文本 |
| AffectedItems | TArray\<FGuid\> | 受影响的物品 InstanceID 列表 |

### EInventoryResult（错误码）

| 值 | 说明 |
|------|------|
| Success | 成功 |
| NoSpace | 空间不足 |
| Overweight | 超重 |
| InvalidItem | 无效物品 |
| InvalidPosition | 无效位置 |
| StackFull | 堆叠已满 |
| NotStackable | 不可堆叠 |

---

## FItemRuntimeData（运行时数据基类）

通过继承扩展物品的运行时动态数据。

### 内置子类

**FFoodRuntimeData**

| 字段 | 类型 | 说明 |
|------|------|------|
| Freshness | float | 新鲜度 (0-100) |
| Temperature | float | 温度 |

**FLiquidRuntimeData**

| 字段 | 类型 | 说明 |
|------|------|------|
| CurrentVolume | float | 当前液体体积 |
| Temperature | float | 温度 |

用户可以继承 `FItemRuntimeData` 添加自定义字段（耐久度、附魔等）。

---

## UInventoryComponent API

### 查询类

| 方法 | 返回值 | 说明 |
|------|--------|------|
| `GetItemDefinition(FName ItemID)` | const FInventoryItemDefinition* | 从 DataTable 缓存中查找物品定义 |
| `FindItem(const FGuid& InstanceID)` | FInventoryItemInstance* | 按实例 ID 查找物品 |
| `GetItems()` | const TArray\<FInventoryItemInstance\>& | 获取全部物品 |
| `FindItemsByID(FName ItemRowID)` | TArray\<FGuid\> | 查找所有指定类型的物品 |
| `GetItemCount(FName ItemRowID)` | int32 | 某类物品总数量（跨堆叠） |
| `HasItem(FName ItemRowID)` | bool | 是否至少有一个指定类型物品 |
| `GetCurrentWeight()` | float | 当前总重量 |
| `GetMaxWeight()` | float | 最大承重 |
| `CanCarryWeight(float)` | bool | 能否承载指定额外重量 |
| `IsCellOccupied(FIntPoint Cell)` | bool | 指定格子是否被占用 |
| `CanPlaceItem(Item, Pos, IgnoreIndex)` | bool | 物品能否放在指定位置 |
| `GetItemSize(const FInventoryItemInstance&)` | FIntPoint | 获取物品占格尺寸（处理旋转） |

### 修改类

| 方法 | 返回值 | 说明 |
|------|--------|------|
| `AddItem(Item)` | FInventoryOperationResult | 添加物品（自动堆叠或占格） |
| `RemoveItem(InstanceID)` | FInventoryOperationResult | 移除单个物品 |
| `DropItem(InstanceID)` | FInventoryOperationResult | 丢弃物品（移除 + 触发 OnItemDropped） |
| `MoveItem(InstanceID, Pos, bRotated)` | FInventoryOperationResult | 移动物品到新位置 |
| `RotateItem(InstanceID)` | FInventoryOperationResult | 旋转物品（自动调整位置） |
| `MergeStack(SourceID, TargetID)` | FInventoryOperationResult | 合并两个同类型堆叠 |
| `SplitStack(InstanceID, Amount)` | FInventoryOperationResult | 拆分堆叠 |
| `TransferItem(ID, TargetInv, Pos, bRot)` | FInventoryOperationResult | 跨背包转移 |
| `AutoArrange()` | FInventoryOperationResult | 自动整理（大物品优先） |
| `ResizeInventory(NewSize)` | FInventoryOperationResult | 调整背包大小 |
| `RemoveAll()` | FInventoryOperationResult | 清空所有物品 |
| `RemoveItemsByID(ItemRowID, Count)` | FInventoryOperationResult | 移除指定数量的某类物品 |

### 快捷栏

| 方法 | 返回值 | 说明 |
|------|--------|------|
| `SetQuickSlot(SlotIndex, ItemID)` | bool | 将物品绑定到快捷槽位 |
| `ClearQuickSlot(SlotIndex)` | bool | 清空快捷槽位 |
| `UseQuickSlot(SlotIndex)` | bool | 使用快捷槽位绑定的物品 |
| `UseItem(Item)` | void | 广播使用请求（OnItemUseRequested） |

### 右键菜单

| 方法 | 返回值 | 说明 |
|------|--------|------|
| `GetAvailableActions(InstanceID)` | TArray\<FGameplayTag\> | 查询物品可用操作标签 |
| `ExecuteAction(ActionTag, InstanceID)` | bool | 执行操作（内置标签走 C++，未知标签走 OnCustomAction） |
| `OnCustomAction(ActionTag, InstanceID)` | void | BlueprintImplementableEvent，自定义操作扩展点 |

### 存档

| 方法 | 返回值 | 说明 |
|------|--------|------|
| `SaveToData()` | FInventorySaveData | 导出当前状态 |
| `LoadFromData(Data)` | void | 从存档数据恢复 |

---

## 委托系统

所有委托均为 `BlueprintAssignable`，蓝图可直接绑定。

| 委托 | 参数 | 触发时机 |
|------|------|----------|
| `OnInventoryChanged` | — | 任何修改操作完成后 |
| `OnItemAdded` | const FInventoryItemInstance& | 添加新物品 |
| `OnItemRemoved` | const FGuid& InstanceID | 物品从背包移除 |
| `OnItemDropped` | const FGuid& InstanceID | 物品被丢弃（RemoveItem + 额外广播） |
| `OnItemMoved` | InstanceID, NewPosition, bRotated | 物品移动 |
| `OnItemUpdated` | const FGuid& InstanceID | 物品数据更新（数量、状态） |
| `OnInventoryResized` | — | 背包尺寸改变 |
| `OnWeightChanged` | — | 重量变化 |
| `OnQuickSlotChanged` | — | 快捷栏绑定改变 |
| `OnItemUseRequested` | const FGuid& InstanceID | 请求使用物品 |
| `OnInventoryCleared` | — | 背包被清空（RemoveAll / LoadFromData） |

---

## GameplayTag 系统

### 行为标签

插件在 C++ 中预定义了 4 个操作标签，用户可在编辑器中扩展。编辑器添加的标签会自动出现在右键菜单中。

| Tag | C++ 变量 | 行为 |
|-----|----------|------|
| `库存插件.行为.使用` | `TAG_Inventory_Action_使用` | 广播 OnItemUseRequested |
| `库存插件.行为.丢弃` | `TAG_Inventory_Action_丢弃` | 调用 DropItem |
| `库存插件.行为.旋转` | `TAG_Inventory_Action_旋转` | 调用 RotateItem |
| `库存插件.行为.拆分` | `TAG_Inventory_Action_拆分` | 调用 SplitStack |

编辑器添加的自定义 Tag 会通过 `OnCustomAction` BlueprintImplementableEvent 传递给蓝图处理。

### 物品类型标签

| Tag | C++ 变量 |
|-----|----------|
| `库存插件.类型.消耗品` | `TAG_Inventory_Type_消耗品` |
| `库存插件.类型.装备` | `TAG_Inventory_Type_装备` |
| `库存插件.类型.武器` | `TAG_Inventory_Type_武器` |
| `库存插件.类型.防具` | `TAG_Inventory_Type_防具` |
| `库存插件.类型.材料` | `TAG_Inventory_Type_材料` |
| `库存插件.类型.任务物品` | `TAG_Inventory_Type_任务物品` |
| `库存插件.类型.工具` | `TAG_Inventory_Type_工具` |
| `库存插件.类型.弹药` | `TAG_Inventory_Type_弹药` |

---

## UI 控件

### UInventoryPanelWidget — 背包根面板

组合 Grid + QuickBar + Tooltip + ContextMenu，负责面板打开/关闭。

| 方法 / 属性 | 说明 |
|-------------|------|
| `InitializeInventory(Inventory)` | 绑定单个背包，初始化全部子控件 |
| `InitializeDualInventory(PlayerInv, ExternalInv)` | 双网格模式（玩家 + 外部容器） |
| `ToggleInventory()` | 切换面板显隐 |
| `CloseInventory()` | 关闭面板，恢复游戏输入模式 |

**Blueprint 扩展事件：**
- `OnItemDoubleClicked(ItemID)`：双击物品
- ContextMenu Widget class 可在蓝图中配置

**BindWidget 子控件：**
- `InventoryGrid`：主网格
- `ExternalInventoryGrid`：外部容器网格（双开可见）
- `InventoryTooltipWidget`：悬浮提示
- `GridContainer`：HorizontalBox 网格容器
- `ContextMenu`：右键菜单

### UInventoryGridWidget — 网格控件

绘制格子背景 + 网格线，管理物品控件的创建/更新/移除，处理拖拽放置。

| 属性 / 方法 | 说明 |
|-------------|------|
| `CellSize` | 单格像素尺寸（默认 64） |
| `GridLineThickness` | 网格线粗细（默认 2） |
| `ItemWidgetClass` | ItemWidget 蓝图类 |
| `InitializeGrid(Inventory)` | 绑定背包，绑定委托，首次刷新 |
| `GridToPixel(GridPos)` | 格子坐标 → 像素坐标 |
| `PixelToGrid(PixelPos)` | 像素坐标 → 格子坐标 |

**拖拽预览：** `NativePaint` 在拖拽经过时绘制绿（合法）/ 红（非法）色块，实时显示放置位置。

### UInventoryItemWidget — 物品控件

单个物品格子的显示控件，支持左键拖拽、右键菜单、悬浮提示。

| 方法 | 说明 |
|------|------|
| `InitializeItem(ItemID, Inventory)` | 绑定物品实例 |
| `SetCellSize(Size)` | 设置格子像素尺寸（由 Grid 创建时注入） |
| `Refresh()` | 从 Runtime 拉取最新数据刷新图标和堆叠数量 |

### UInventoryTooltipWidget — 悬浮提示

鼠标停留 1 秒后显示，位置跟随鼠标。

| 方法 | 说明 |
|------|------|
| `SetItem(ItemID, Inventory, MousePos)` | 设置显示物品并定位 |

**BindWidget 子控件：** `ItemName`、`ItemWeight`、`ItemDescription`

**扩展事件：** `OnRuntimeDataDisplay(ItemID)`：BlueprintImplementableEvent，显示 RuntimeData

### UQuickBarWidget — 快捷栏容器

管理 6 个快捷槽位，支持拖入绑定，自动跟随物品移除。

| 方法 / 属性 | 说明 |
|-------------|------|
| `InitializeQuickBar(Inventory)` | 绑定背包并创建槽位 |
| `RefreshAllSlots()` | 刷新全部槽位显示 |
| `QuickSlotClass` | QuickSlot 蓝图类 |
| `SlotCount` | 槽位数量（默认 6） |

**BindWidget 子控件：** `SlotPanel` (HorizontalBox)

### UQuickSlotWidget — 快捷栏槽位

单个快捷栏槽位的显示控件。

| 方法 | 说明 |
|------|------|
| `InitializeSlot(SlotIndex, Inventory)` | 绑定槽位索引和背包 |
| `Refresh()` | 从 Runtime 刷新图标和数量 |

**BindWidget 子控件：** `ItemIcon`、`HotkeyText`、`StackCount`

---

## UInventoryInteractionComponent — 交互组件

挂载在 PlayerController 上，负责 UI 生命周期管理、输入绑定、背包会话。

| 方法 | 说明 |
|------|------|
| `OpenPlayerInventory()` | 打开玩家背包 |
| `OpenExternalInventory(ExternalInv)` | 打开外部容器（双网格） |
| `CloseInventory()` | 关闭所有面板 |
| `ToggleInventory()` | 切换背包显隐 |
| `IsInventoryOpen()` | 背包是否打开 |
| `GetPlayerInventory()` | 获取当前玩家背包 |
| `GetExternalInventory()` | 获取当前外部容器 |

**配置属性：**
- `InventoryWidgetClass`：PanelWidget 蓝图类
- `QuickBarWidgetClass`：QuickBarWidget 蓝图类
- `IA_ToggleInventory` ~ `IA_QuickSlot6`：EnhancedInput Action 绑定

**扩展事件：**
- `OnInventoryOpened()` / `OnInventoryClosed()`
- `OnExternalInventoryOpened(ExternalInv)`
- `OnQuickSlotUsed(SlotIndex)`

---

## IInventoryProviderInterface — 背包提供者接口

任何拥有背包的 Actor 实现此接口暴露 `UInventoryComponent`。

| 方法 | 说明 |
|------|------|
| `GetInventoryComponent()` | 返回此 Actor 的背包组件 |

典型实现：`return GetComponentByClass<UInventoryComponent>();`

---

## UInventoryDragDropOperation — 拖拽操作

继承 `UDragDropOperation`，存储拖拽过程中的临时状态。

| 属性 / 方法 | 说明 |
|-------------|------|
| `ItemID` | 被拖拽物品 InstanceID |
| `bRotated` | 旋转状态 |
| `NormalizedGrabPoint` | 归一化抓取点 |
| `SourceInventory` | 来源背包 |
| `RequestRotate()` | 切换旋转并广播 OnRotated |
| `GetCurrentDragOffset(ItemPixelSize)` | 计算鼠标到物品左上角的像素偏移 |

---

## 快速开始

### 1. 创建 DataTable

在 Content Browser 中创建 DataTable，行结构选 `FInventoryItemDefinition`，填入物品行数据。

### 2. 配置背包组件

在角色/容器的 Blueprint 中添加 `UInventoryComponent`，设置 `ItemDefinitionTable` 指向步骤 1 的 DataTable，调整 `GridSize` 和 `MaxWeight`。

角色需实现 `IInventoryProviderInterface`：
```
事件 GetInventoryComponent → return InventoryComponent
```

### 3. 配置交互组件

在 PlayerController 的 Blueprint 中添加 `UInventoryInteractionComponent`，配置：
- `InventoryWidgetClass` → WBP_InventoryPanel
- `QuickBarWidgetClass` → WBP_QuickBar
- 各 `IA_*` 输入 Action

### 4. 创建 Widget 蓝图

创建蓝图 Widget 继承对应 C++ 基类：
- `WBP_InventoryPanel` 继承 `UInventoryPanelWidget`
- `WBP_InventoryGrid` 继承 `UInventoryGridWidget`
- `WBP_InventoryItem` 继承 `UInventoryItemWidget`
- `WBP_InventoryTooltip` 继承 `UInventoryTooltipWidget`
- `WBP_ContextMenu` 继承 `UInventoryContextMenuWidget`
- `WBP_QuickBar` 继承 `UQuickBarWidget`
- `WBP_QuickSlot` 继承 `UQuickSlotWidget`

在蓝图中绑定对应的 Widget 命名的控件到 BindWidget。

### 5. 映射输入

创建 Input Mapping Context，绑定：
- Tab → IA_ToggleInventory
- Escape → IA_CloseInventory
- R → IA_RotateItem
- 1-6 → IA_QuickSlot1~6

---

## 常见扩展场景

### 使用物品

蓝图中绑定 `OnItemUseRequested` 委托：

1. 获取 `InstanceID`
2. 通过 `FindItem` 获取物品实例
3. 读取 `ItemID` 和 `RuntimeData` 决定行为（回血、上 Buff）
4. 调用 `RemoveItem` 或 `RemoveItemsByID` 消耗物品

### 丢弃物品到世界

蓝图中绑定 `OnItemDropped` 委托：

1. 获取 `InstanceID`
2. 在角色位置生成物品 Actor/拾取物
3. 将物品数据传递给世界 Actor

### 合成系统

利用现有 API 实现：

1. `FindItemsByID` 检查材料是否充足
2. `RemoveItemsByID` 扣除材料
3. 构造 `FInventoryItemInstance` 产物
4. `AddItem` 添加产物

### 自定义操作菜单

1. 在物品定义 `可用操作` 中添加自定义 GameplayTag
2. 蓝图中重载 `OnCustomAction(ActionTag, InstanceID)`
3. 根据 ActionTag 执行对应逻辑

### 存档

```
// 存档
FInventorySaveData SaveData = Inventory->SaveToData();
// 写入 USaveGame 对象

// 读档
FInventorySaveData LoadedData = /* 从 USaveGame 读取 */;
Inventory->LoadFromData(LoadedData);
```

---

## 注意事项

- `FInventoryItemInstance` 的 `InstanceID` 是物品的唯一标识，拖拽/快捷栏/菜单均以此为键，不要手动修改
- `OccupancyMap` 在每次修改操作后全量重建，不依赖增量同步
- `RemoveAll` 和 `RemoveItemsByID` 会逐个广播委托，确保 UI 完全同步
- 快捷栏自动监听 `OnItemRemoved`，物品被移除时会自动清空引用的槽位
- 拖拽期间按 R 键旋转，预览色块实时更新合法性判定
- `ItemDefinitionTable` 的查找结果会被 `DefinitionCache` 缓存，外部修改 DataTable 后需清缓存
