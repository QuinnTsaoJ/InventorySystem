// 背包提供者接口 —— 任何拥有背包的 Actor 通过此接口暴露 InventoryComponent
//
// 实现者：Character / Pawn / LootContainer / StorageActor / Vehicle
// 调用者：PlayerController（获取背包以创建 UI）、其他背包（TransferItem）
//
// 极简设计：只有一个方法。不包含 UI 控制、面板开关等——那些属于 Controller 层。
//
// InventorySystem Plugin

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryProviderInterface.generated.h"

class UInventoryComponent;

UINTERFACE(MinimalAPI, BlueprintType)
class UInventoryProviderInterface : public UInterface
{
	GENERATED_BODY()
};

class INVENTORYSYSTEM_API IInventoryProviderInterface
{
	GENERATED_BODY()

public:
	/**
	 * 获取此 Actor 拥有的背包组件。
	 * 典型实现：return GetComponentByClass<UInventoryComponent>();
	 * @return 背包组件指针，无背包时返回 nullptr
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory|Provider")
	UInventoryComponent* GetInventoryComponent() const;
};
