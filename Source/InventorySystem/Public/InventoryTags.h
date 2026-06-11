// 背包操作 GameplayTag 定义 —— 默认内置标签，用户可在编辑器中扩展
// InventorySystem Plugin

#pragma once

#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

// C++ 预定义操作标签，在 InventoryTags.cpp 中注册
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Action_使用);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Action_丢弃);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Action_旋转);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Action_拆分);
