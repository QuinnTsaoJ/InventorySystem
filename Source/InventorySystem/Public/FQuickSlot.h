#pragma once

#include "CoreMinimal.h"
#include "FQuickSlot.generated.h"

USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FQuickSlot
{
	GENERATED_BODY()

public:

	/** 快捷栏绑定的物品实例标识 */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Inventory|QuickSlot", meta=(ToolTip="快捷栏绑定的物品实例标识"))
	FGuid ItemInstanceID;
};
