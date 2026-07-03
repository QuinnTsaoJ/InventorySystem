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

	UPROPERTY(SaveGame,EditAnywhere, BlueprintReadWrite, DisplayName = "新鲜度")
	float Freshness = 100.f;

	UPROPERTY(SaveGame,EditAnywhere, BlueprintReadWrite, DisplayName = "温度")
	float Temperature = 25.f;
	
	
};


USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FLiquidRuntimeData : public FItemRuntimeData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame,EditAnywhere, BlueprintReadWrite, DisplayName = "液体体积")
	float CurrentVolume = 1.f;

	UPROPERTY(SaveGame,EditAnywhere, BlueprintReadWrite, DisplayName = "温度")
	float Temperature = 25.f;
};
