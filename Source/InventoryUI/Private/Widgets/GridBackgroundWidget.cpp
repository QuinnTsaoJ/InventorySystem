// 格子背景控件实现
// InventorySystem Plugin

#include "Widgets/GridBackgroundWidget.h"
#include "Styling/SlateStyle.h"

void UGridBackgroundWidget::SetGridParameters(FIntPoint InGridSize, float InCellSize, float InLineThickness)
{
	GridSize = InGridSize;
	CellSize = InCellSize;
	GridLineThickness = InLineThickness;
}

int32 UGridBackgroundWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (GridSize.X <= 0 || GridSize.Y <= 0)
	{
		return LayerId;
	}

	const FVector2f GridPixelSize(GridSize.X * CellSize, GridSize.Y * CellSize);

	// 1. 黑色背景
	FSlateDrawElement::MakeBox(
		OutDrawElements, LayerId++,
		AllottedGeometry.ToPaintGeometry(GridPixelSize, FSlateLayoutTransform()),
		FCoreStyle::Get().GetBrush("WhiteBrush"),
		ESlateDrawEffect::None,
		FLinearColor(0.02f, 0.02f, 0.02f, 1.0f)
	);

	// 2. 网格线
	const FLinearColor LineColor(0.12f, 0.12f, 0.12f, 0.6f);

	for (int32 x = 0; x <= GridSize.X; ++x)
	{
		const float XPos = x * CellSize;
		FSlateDrawElement::MakeBox(
			OutDrawElements, LayerId,
			AllottedGeometry.ToPaintGeometry(FVector2f(GridLineThickness, GridPixelSize.Y), FSlateLayoutTransform(FVector2f(XPos, 0.0f))),
			FCoreStyle::Get().GetBrush("WhiteBrush"),
			ESlateDrawEffect::None, LineColor
		);
	}
	++LayerId;

	for (int32 y = 0; y <= GridSize.Y; ++y)
	{
		const float YPos = y * CellSize;
		FSlateDrawElement::MakeBox(
			OutDrawElements, LayerId,
			AllottedGeometry.ToPaintGeometry(FVector2f(GridPixelSize.X, GridLineThickness), FSlateLayoutTransform(FVector2f(0.0f, YPos))),
			FCoreStyle::Get().GetBrush("WhiteBrush"),
			ESlateDrawEffect::None, LineColor
		);
	}

	return LayerId + 1;
}
