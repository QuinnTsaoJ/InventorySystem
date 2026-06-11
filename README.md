# InventorySystem

UE5 网格背包插件，提供物品数据管理、格子占用、拖拽交互、快捷栏、右键菜单和存档支持。插件负责数据与 UI 基础设施，玩法逻辑（使用效果、合成、世界交互）由游戏层通过委托和 GameplayTag 扩展。

## 架构

插件分为两个模块：

**InventorySystem (Runtime)** — 纯数据与逻辑，不依赖 Slate/UMG
- `UInventoryComponent`：ActorComponent，挂载于角色或容器，管理物品增删移转、格子占用、重量、快捷栏
- `FInventoryItemDefinition`：DataTable 行结构，定义物品模板（尺寸、重量、图标、堆叠、旋转、可用操作 Tag）
- `FInventoryItemInstance`：运行时物品实例（唯一 GUID、数量、位置、旋转态、RuntimeData）
- `FInventorySaveData`：存档结构体，`SaveGame` 标记，游戏层负责持久化

**InventoryUI (Runtime)** — 基于 UMG/Slate 的 UI 控件
- `UInventoryPanelWidget`：背包根面板，组合网格 + 快捷栏 + 悬浮提示 + 右键菜单
- `UInventoryGridWidget`：网格控件，渲染格子线、物品图标、拖拽预览色块
- `UInventoryItemWidget`：单个物品控件，支持拖拽、悬浮提示、右键菜单
- `UQuickBarWidget` / `UQuickSlotWidget`：快捷栏，支持拖入绑定、热键使用、自动跟随移除
- `UInventoryInteractionComponent`：挂载 PlayerController，管理输入、面板生命周期、会话

## 功能

- **网格背包**：物品按格占位，自动碰撞检测，支持旋转、堆叠、拆分、合并
- **拖拽交互**：同背包移动 / 跨背包转移，R 键旋转，实时放置预览（绿/红）
- **快捷栏**：数字键 1-6 使用，拖入绑定，物品移除时自动清空对应槽位
- **右键菜单**：GameplayTag 驱动，内置 使用/丢弃/旋转/拆分，蓝图可扩展自定义操作
- **悬浮提示**：鼠标停留 1 秒显示物品详情，支持 RuntimeData 扩展（新鲜度、温度等）
- **双容器模式**：同时显示玩家背包 + 外部容器（战利品箱）
- **物品定义**：DataTable 驱动，编辑器配置图标、尺寸、堆叠上限、可用操作 Tag
- **委托系统**：10 个 BlueprintAssignable 委托，覆盖增删移变化丢弃清空快件栏使用请求
- **存档**：`SaveToData()` / `LoadFromData()` 导出/恢复全量状态

## 快速开始

1. 创建 DataTable，行结构选 `FInventoryItemDefinition`，填入物品数据
2. 角色/容器添加 `UInventoryComponent`，实现 `IInventoryProviderInterface` 暴露背包
3. PlayerController 添加 `UInventoryInteractionComponent`，配置 Widget 蓝图类和输入 Action
4. 创建蓝图 Widget 继承 C++ 基类并绑定子控件（Grid、Tooltip、QuickBar、ContextMenu 等）
5. 通过 `InventoryComponent` 的 API 操作物品，绑定委托响应 UI 刷新

## 扩展点

- **自定义操作**：在物品定义 `可用操作` 中添加 GameplayTag，蓝图重载 `OnCustomAction` 处理
- **使用物品**：绑定 `OnItemUseRequested` 委托，读取物品 RuntimeData 决定行为
- **丢弃到世界**：绑定 `OnItemDropped` 委托，获取物品位置生成 Actor
- **RuntimeData**：继承 `FItemRuntimeData` 添加自定义字段（耐久度、附魔等），蓝图 `OnRuntimeDataDisplay` 实现 UI
- **存档**：调用 `SaveToData()` 获取数据，存入 `USaveGame`；读档时取出调用 `LoadFromData()`
