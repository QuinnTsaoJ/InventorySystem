#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "RuntimeData.h"
#include "StructUtils/InstancedStruct.h"
#include "DeferredInstancedStruct.h"
#include "FInventoryItemDefinition.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FInventoryItemDefinition : public FTableRowBase
{
	GENERATED_BODY()

public:

	// 基础信息
	/** 物品在界面中显示的名称 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="名称", Category="Inventory|Definition", meta=(ToolTip="物品在界面中显示的名称"))
	FText Name;

	/** 物品在提示框中显示的描述 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="描述", Category="Inventory|Definition", meta=(ToolTip="物品在提示框中显示的描述"))
	FText Description;

	/** 物品的默认图标 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="图标", Category="Inventory|Definition", meta=(ToolTip="物品的默认图标"))
	TObjectPtr<UTexture2D> Icon;

	/** 物品旋转后显示的图标 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="旋转图标", Category="Inventory|Definition", meta=(ToolTip="物品旋转后显示的图标"))
	TObjectPtr<UTexture2D> IconRotated;

	// Grid尺寸
	/** 物品占用的网格尺寸 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="大小", Category="Inventory|Definition", meta=(ToolTip="物品占用的网格尺寸"))
	FIntPoint Size = FIntPoint(1, 1);

	// 重量
	/** 单个物品的重量 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="重量", Category="Inventory|Definition", meta=(ToolTip="单个物品的重量"))
	float Weight = 0.f;

	// 是否允许旋转
	/** 是否允许在网格中旋转物品 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="旋转", Category="Inventory|Definition", meta=(ToolTip="是否允许在网格中旋转物品"))
	bool bCanRotate = true;

	// 是否允许堆叠
	/** 是否允许同类物品堆叠 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="堆叠", Category="Inventory|Definition", meta=(ToolTip="是否允许同类物品堆叠"))
	bool bStackable = false;

	// 最大堆叠数量
	/** 单格可容纳的最大物品数量 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="最大堆叠数量", Category="Inventory|Definition", meta=(ToolTip="单格可容纳的最大物品数量"))
	int32 MaxStackSize = 1;

	// 是否需要实例数据
	/** 每个物品是否需要独立的运行时数据 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="需要实例数据", Category="Inventory|Definition", meta=(ToolTip="每个物品是否需要独立的运行时数据"))
	bool bUseInstanceData = false;

	// GameplayTag
	/** 用于标识物品类型的标签 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="物品标签", Category="Inventory|Definition", meta=(ToolTip="用于标识物品类型的标签"))
	FGameplayTagContainer ItemTags;

	/** 右键菜单可用操作标签，编辑器中选择或添加自定义 Tag 即可扩展菜单项 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="可用操作", Category="Inventory|Definition", meta=(ToolTip="右键菜单可用操作标签，编辑器中选择或添加自定义 Tag 即可扩展菜单项"))
	FGameplayTagContainer EnabledActions;

	// RuntimeData类型（软路径持久化，支持跨插件模块定义的派生结构体，重启不丢失）
	/** 每个实例使用的运行时数据类型及初始值 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="RuntimeData类型", Category="Inventory|Definition", meta=(ToolTip="每个实例使用的运行时数据类型及初始值"))
	FDeferredInstancedStruct RuntimeDataType;
};
