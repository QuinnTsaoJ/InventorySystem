// 格子背景控件 —— 负责绘制背包的黑色背景 + 网格线
// 作为 ItemCanvas 的子控件，ZOrder 最低，在所有物品下方
// InventoryUI Module

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GridBackgroundWidget.generated.h"

UCLASS()
class INVENTORYUI_API UGridBackgroundWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 设置要绘制的格子参数 */
	void SetGridParameters(FIntPoint InGridSize, float InCellSize, float InLineThickness);

protected:
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	FIntPoint GridSize = FIntPoint::ZeroValue;
	float CellSize = 64.f;
	float GridLineThickness = 2.f;
};
