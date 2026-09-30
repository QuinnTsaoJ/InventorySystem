#pragma once

#include "CoreMinimal.h"
#include "RuntimeData.generated.h"

USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemRuntimeData
{
	GENERATED_BODY()
};


USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FFoodRuntimeData : public FItemRuntimeData
{
	GENERATED_BODY()

	/** 食物当前的新鲜度 */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, DisplayName = "新鲜度", Category = "Inventory|Food", meta=(ToolTip="食物当前的新鲜度"))
	float Freshness = 100.f;

	/** 食物当前的温度 */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, DisplayName = "温度", Category = "Inventory|Food", meta=(ToolTip="食物当前的温度"))
	float Temperature = 25.f;


};


USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FLiquidRuntimeData : public FItemRuntimeData
{
	GENERATED_BODY()

	/** 液体当前的体积 */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, DisplayName = "液体体积", Category = "Inventory|Liquid", meta=(ToolTip="液体当前的体积"))
	float CurrentVolume = 1.f;

	/** 液体当前的温度 */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, DisplayName = "温度", Category = "Inventory|Liquid", meta=(ToolTip="液体当前的温度"))
	float Temperature = 25.f;
};
