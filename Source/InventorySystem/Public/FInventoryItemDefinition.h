#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RuntimeData.h"
#include "InstancedStruct.h"
#include "StructUtils/InstancedStruct.h"
#include "FInventoryItemDefinition.generated.h"

USTRUCT(BlueprintType)
struct FInventoryItemDefinition : public FTableRowBase
{
	GENERATED_BODY()

public:

	// 基础信息
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,DisplayName="名称")
	FText Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,DisplayName="描述")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,DisplayName="图标")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,DisplayName="旋转图标")
	TObjectPtr<UTexture2D> IconRotated;

	// Grid尺寸
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,DisplayName="大小")
	FIntPoint Size = FIntPoint(1, 1);

	// 重量
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,DisplayName="重量")
	float Weight = 0.f;

	// 是否允许旋转
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,DisplayName="旋转")
	bool bCanRotate = true;

	// 是否允许堆叠
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,DisplayName="堆叠")
	bool bStackable = false;

	// 最大堆叠数量
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,DisplayName="最大堆叠数量")
	int32 MaxStackSize = 1;

	// 是否需要实例数据
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,DisplayName="需要实例数据")
	bool bUseInstanceData = false;

	// GameplayTag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,DisplayName="物品标签")
	FGameplayTagContainer ItemTags;

	/** 右键菜单可用操作标签，编辑器中选择或添加自定义 Tag 即可扩展菜单项 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="可用操作")
	FGameplayTagContainer EnabledActions;

	// RuntimeData类型
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,DisplayName="RuntimeData类型")
	TInstancedStruct<FItemRuntimeData> RuntimeDataType;
};