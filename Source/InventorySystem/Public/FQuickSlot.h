#pragma once

#include "CoreMinimal.h"
#include "FQuickSlot.generated.h"

USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FQuickSlot
{
	GENERATED_BODY()

public:

	UPROPERTY(SaveGame,EditAnywhere, BlueprintReadWrite)
	FGuid ItemInstanceID;
};
