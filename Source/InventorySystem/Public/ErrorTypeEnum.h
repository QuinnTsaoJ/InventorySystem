#pragma once

UENUM(BlueprintType)
enum class EInventoryResult : uint8
{
	Success UMETA(DisplayName="成功"),
	NoSpace UMETA(DisplayName="失败"),
	Overweight UMETA(DisplayName="超重"),
	InvalidItem UMETA(DisplayName="无效物品"),
	InvalidPosition UMETA(DisplayName="无效位置"),
	StackFull UMETA(DisplayName="堆叠已满"),
	NotStackable UMETA(DisplayName="不可堆叠"),
};