// 背包系统核心组件实现
// InventorySystem Plugin

#include "InventoryComponent.h"
#include "InventoryTags.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "UObject/ConstructorHelpers.h"

UInventoryComponent::UInventoryComponent()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> ItemTableFinder(
		TEXT("/Script/Engine.DataTable'/InventorySystem/Data/ItemDataTable.ItemDataTable'"));
	if (ItemTableFinder.Succeeded())
	{
		ItemDefinitionTable = ItemTableFinder.Object;
	}

	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	RebuildOccupancyMap();
	RecalculateWeight();
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

//=============================================================================
// 查询类 API
//=============================================================================

const FInventoryItemDefinition* UInventoryComponent::GetItemDefinition(FName ItemID) const
{
	if (const FInventoryItemDefinition* const* Cached = DefinitionCache.Find(ItemID))
	{
		return *Cached;
	}

	if (!ItemDefinitionTable)
	{
		return nullptr;
	}

	const FInventoryItemDefinition* Def = ItemDefinitionTable->FindRow<FInventoryItemDefinition>(ItemID, TEXT("GetItemDefinition"));
	if (Def)
	{
		DefinitionCache.Add(ItemID, Def);
	}
	return Def;
}

FInventoryItemInstance* UInventoryComponent::FindItem(const FGuid& InstanceID)
{
	for (FInventoryItemInstance& Item : Items)
	{
		if (Item.InstanceID == InstanceID)
		{
			return &Item;
		}
	}
	return nullptr;
}

const TArray<FInventoryItemInstance>& UInventoryComponent::GetItems() const
{
	return Items;
}

float UInventoryComponent::GetCurrentWeight() const
{
	return CurrentWeight;
}

float UInventoryComponent::GetMaxWeight() const
{
	return MaxWeight;
}

bool UInventoryComponent::CanCarryWeight(float AdditionalWeight) const
{
	return (CurrentWeight + AdditionalWeight) <= MaxWeight;
}

TArray<FGuid> UInventoryComponent::FindItemsByID(FName ItemRowID) const
{
	TArray<FGuid> Result;
	for (const FInventoryItemInstance& Item : Items)
	{
		if (Item.ItemID == ItemRowID)
		{
			Result.Add(Item.InstanceID);
		}
	}
	return Result;
}

int32 UInventoryComponent::GetItemCount(FName ItemRowID) const
{
	int32 Count = 0;
	for (const FInventoryItemInstance& Item : Items)
	{
		if (Item.ItemID == ItemRowID)
		{
			Count += Item.Quantity;
		}
	}
	return Count;
}

bool UInventoryComponent::HasItem(FName ItemRowID) const
{
	return GetItemCount(ItemRowID) > 0;
}

//=============================================================================
// OccupancyMap 与空间检测
//=============================================================================

bool UInventoryComponent::IsCellOccupied(FIntPoint Cell) const
{
	if (Cell.X < 0 || Cell.Y < 0 || Cell.X >= GridSize.X || Cell.Y >= GridSize.Y)
	{
		return false;
	}

	const int32 Index = Cell.Y * GridSize.X + Cell.X;
	return OccupancyMap.IsValidIndex(Index) && OccupancyMap[Index].IsValid();
}

bool UInventoryComponent::CanPlaceItem(const FInventoryItemInstance& Item, FIntPoint Position, int32 IgnoreIndex) const
{
	const FIntPoint Size = GetItemSize(Item);

	if (Position.X < 0 || Position.Y < 0 ||
		Position.X + Size.X > GridSize.X ||
		Position.Y + Size.Y > GridSize.Y)
	{
		return false;
	}

	for (int32 y = 0; y < Size.Y; ++y)
	{
		for (int32 x = 0; x < Size.X; ++x)
		{
			const int32 CellIndex = (Position.Y + y) * GridSize.X + (Position.X + x);
			if (!OccupancyMap.IsValidIndex(CellIndex))
			{
				return false;
			}

			const FGuid& OccupantID = OccupancyMap[CellIndex];
			if (OccupantID.IsValid())
			{
				// 忽略指定索引的物品（移动/旋转时允许自重叠）
				if (IgnoreIndex >= 0 && Items.IsValidIndex(IgnoreIndex) && Items[IgnoreIndex].InstanceID == OccupantID)
				{
					continue;
				}

				if (OccupantID != Item.InstanceID)
				{
					return false;
				}
			}
		}
	}

	return true;
}

bool UInventoryComponent::FindAvailablePosition(const FInventoryItemInstance& Item, FIntPoint& OutPosition) const
{
	for (int32 y = 0; y < GridSize.Y; ++y)
	{
		for (int32 x = 0; x < GridSize.X; ++x)
		{
			const FIntPoint Candidate(x, y);
			if (CanPlaceItem(Item, Candidate))
			{
				OutPosition = Candidate;
				return true;
			}
		}
	}
	return false;
}

//=============================================================================
// 修改类 API
//=============================================================================

FInventoryOperationResult UInventoryComponent::AddItem(const FInventoryItemInstance& InItem)
{
	FInventoryOperationResult Result;

	const FInventoryItemDefinition* Def = GetItemDefinition(InItem.ItemID);
	if (!Def)
	{
		Result.ResultCode = EInventoryResult::InvalidItem;
		Result.Message = FText::FromString(TEXT("无效物品ID"));
		return Result;
	}

	if (!CanCarryWeight(Def->Weight * InItem.Quantity))
	{
		Result.ResultCode = EInventoryResult::Overweight;
		Result.Message = FText::FromString(TEXT("超重，无法添加"));
		return Result;
	}

	int32 RemainingQuantity = InItem.Quantity;

	if (Def->bStackable && !Def->bUseInstanceData)
	{
		for (const FInventoryItemInstance& ExistingItem : Items)
		{
			if (ExistingItem.ItemID == InItem.ItemID && ExistingItem.Quantity < Def->MaxStackSize)
			{
				const int32 Room = Def->MaxStackSize - ExistingItem.Quantity;
				RemainingQuantity -= FMath::Min(Room, RemainingQuantity);
				if (RemainingQuantity <= 0)
				{
					break;
				}
			}
		}
	}

	if (RemainingQuantity > 0)
	{
		FInventoryItemInstance TempItem;
		TempItem.ItemID = InItem.ItemID;
		TempItem.Quantity = RemainingQuantity;
		TempItem.bRotated = InItem.bRotated;
		if (InItem.RuntimeData.IsValid())
		{
			TempItem.RuntimeData = InItem.RuntimeData;
		}

		FIntPoint DummyPosition;
		if (!FindAvailablePosition(TempItem, DummyPosition))
		{
			Result.Message = FText::FromString(TEXT("空间不足"));
			Result.ResultCode = EInventoryResult::NoSpace;
			return Result;
		}
	}

	RemainingQuantity = InItem.Quantity;

	if (Def->bStackable && !Def->bUseInstanceData)
	{
		for (FInventoryItemInstance& ExistingItem : Items)
		{
			if (ExistingItem.ItemID != InItem.ItemID || ExistingItem.Quantity >= Def->MaxStackSize)
			{
				continue;
			}

			const int32 Room = Def->MaxStackSize - ExistingItem.Quantity;
			const int32 ToAdd = FMath::Min(Room, RemainingQuantity);
			ExistingItem.Quantity += ToAdd;
			RemainingQuantity -= ToAdd;

			Result.AffectedItems.Add(ExistingItem.InstanceID);
			OnItemUpdated.Broadcast(ExistingItem.InstanceID);

			if (RemainingQuantity <= 0)
			{
				break;
			}
		}
	}

	if (RemainingQuantity > 0)
	{
		FInventoryItemInstance NewItem;
		NewItem.ItemID = InItem.ItemID;
		NewItem.InstanceID = InItem.InstanceID.IsValid() ? InItem.InstanceID : FGuid::NewGuid();
		NewItem.Quantity = RemainingQuantity;
		NewItem.bRotated = InItem.bRotated;
		if (InItem.RuntimeData.IsValid())
		{
			NewItem.RuntimeData = InItem.RuntimeData;
		}

		FIntPoint NewPosition;
		FindAvailablePosition(NewItem, NewPosition);
		NewItem.Position = NewPosition;
		Items.Add(NewItem);

		Result.AffectedItems.Add(NewItem.InstanceID);
		OnItemAdded.Broadcast(NewItem);
	}

	RebuildOccupancyMap();
	RecalculateWeight();
	OnWeightChanged.Broadcast();
	OnInventoryChanged.Broadcast();

	Result.bSuccess = true;
	return Result;
}

FInventoryOperationResult UInventoryComponent::RemoveItem(const FGuid& InstanceID)
{
	FInventoryOperationResult Result;

	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (Items[i].InstanceID == InstanceID)
		{
			Items.RemoveAt(i);
			RebuildOccupancyMap();
			RecalculateWeight();

			Result.bSuccess = true;
			Result.AffectedItems.Add(InstanceID);
			OnItemRemoved.Broadcast(InstanceID);
			OnWeightChanged.Broadcast();
			OnInventoryChanged.Broadcast();
			return Result;		
		}
	}

	Result.Message = FText::FromString(TEXT("物品不存在"));
		Result.ResultCode = EInventoryResult::InvalidItem;
	return Result;
}

FInventoryOperationResult UInventoryComponent::DropItem(const FGuid& InstanceID)
{
	FInventoryOperationResult Result = RemoveItem(InstanceID);
	if (Result.bSuccess)
	{
		OnItemDropped.Broadcast(InstanceID);
	}
	return Result;
}

FInventoryOperationResult UInventoryComponent::MoveItem(const FGuid& InstanceID, FIntPoint NewPosition, bool bRotated)
{
	FInventoryOperationResult Result;

	const int32 ItemIndex = Items.IndexOfByPredicate([&](const FInventoryItemInstance& Item)
	{
		return Item.InstanceID == InstanceID;
	});

	if (ItemIndex == INDEX_NONE)
	{
		Result.Message = FText::FromString(TEXT("物品不存在"));
		Result.ResultCode = EInventoryResult::InvalidItem;
		return Result;
	}

	const bool bOldRotated = Items[ItemIndex].bRotated;
	Items[ItemIndex].bRotated = bRotated;

	if (!CanPlaceItem(Items[ItemIndex], NewPosition, ItemIndex))
	{
		Items[ItemIndex].bRotated = bOldRotated;
		Result.Message = FText::FromString(TEXT("目标位置不可用"));
		Result.ResultCode = EInventoryResult::InvalidPosition;
		return Result;
	}

	Items[ItemIndex].Position = NewPosition;
	RebuildOccupancyMap();

	Result.bSuccess = true;
	Result.AffectedItems.Add(InstanceID);
	OnItemMoved.Broadcast(InstanceID, NewPosition, bRotated);
	OnInventoryChanged.Broadcast();

	return Result;
}

FInventoryOperationResult UInventoryComponent::RotateItem(const FGuid& InstanceID)
{
	FInventoryOperationResult Result;

	FInventoryItemInstance* Item = FindItem(InstanceID);
	if (!Item)
	{
		Result.Message = FText::FromString(TEXT("物品不存在"));
		Result.ResultCode = EInventoryResult::InvalidItem;
		return Result;
	}

	const FInventoryItemDefinition* Def = GetItemDefinition(Item->ItemID);
	if (!Def || !Def->bCanRotate)
	{
		Result.Message = FText::FromString(TEXT("该物品不可旋转"));
		Result.ResultCode = EInventoryResult::InvalidItem;
		return Result;
	}

	Item->bRotated = !Item->bRotated;

	while (!CanPlaceItem(*Item, Item->Position))
	{
		/*由于旋转方向的问题物体的位移可能会在X的+轴上与y轴的+轴上多出一格，所以这里先检测以下三个位置 */
		FIntPoint NewPosition = Item->Position+FIntPoint{0,-1};
		if (CanPlaceItem(*Item, NewPosition)) {
			Item->Position = NewPosition;
			break;
		}
		NewPosition = Item->Position+FIntPoint{-1,0};
		if (CanPlaceItem(*Item, NewPosition)) {
			Item->Position = NewPosition;
			break;
		}
		NewPosition = Item->Position+FIntPoint{-1,-1};
		if (CanPlaceItem(*Item, NewPosition)) {
			Item->Position = NewPosition;
			break;
		}
		
		if (!FindAvailablePosition(*Item, NewPosition))
		{
			Item->bRotated = !Item->bRotated;
			Result.Message = FText::FromString(TEXT("旋转后无处可放"));
			Result.ResultCode = EInventoryResult::NoSpace;
			return Result;
		}
		Item->Position = NewPosition;
		break;
	}

	RebuildOccupancyMap();

	Result.bSuccess = true;
	Result.AffectedItems.Add(InstanceID);
	OnItemUpdated.Broadcast(InstanceID);
	OnInventoryChanged.Broadcast();

	return Result;
}

FInventoryOperationResult UInventoryComponent::MergeStack(const FGuid& SourceItemID, const FGuid& TargetItemID)
{
	FInventoryOperationResult Result;

	if (SourceItemID == TargetItemID)
	{
		Result.Message = FText::FromString(TEXT("不能合并到自身"));
		Result.ResultCode = EInventoryResult::InvalidItem;
		return Result;
	}

	FInventoryItemInstance* SourceItem = FindItem(SourceItemID);
	FInventoryItemInstance* TargetItem = FindItem(TargetItemID);
	if (!SourceItem || !TargetItem)
	{
		Result.Message = FText::FromString(TEXT("物品不存在"));
		Result.ResultCode = EInventoryResult::InvalidItem;
		return Result;
	}

	if (SourceItem->ItemID != TargetItem->ItemID)
	{
		Result.Message = FText::FromString(TEXT("物品类型不同，无法合并"));
		Result.ResultCode = EInventoryResult::NotStackable;
		return Result;
	}

	const FInventoryItemDefinition* Def = GetItemDefinition(SourceItem->ItemID);
	if (!Def || !Def->bStackable)
	{
		Result.Message = FText::FromString(TEXT("该物品不可堆叠"));
		Result.ResultCode = EInventoryResult::NotStackable;
		return Result;
	}

	if (Def->bUseInstanceData)
	{
		Result.Message = FText::FromString(TEXT("具有实例数据的物品不可堆叠"));
		Result.ResultCode = EInventoryResult::NotStackable;
		return Result;
	}

	const int32 Room = Def->MaxStackSize - TargetItem->Quantity;
	if (Room <= 0)
	{
		Result.Message = FText::FromString(TEXT("目标堆叠已满"));
		Result.ResultCode = EInventoryResult::StackFull;
		return Result;
	}

	const int32 ToAdd = FMath::Min(Room, SourceItem->Quantity);
	TargetItem->Quantity += ToAdd;
	SourceItem->Quantity -= ToAdd;

	if (SourceItem->Quantity <= 0)
	{
		// 源物品耗尽，移除
		RemoveItem(SourceItemID);
	}

	RebuildOccupancyMap();
	RecalculateWeight();

	Result.bSuccess = true;
	Result.AffectedItems.Add(SourceItemID);
	Result.AffectedItems.Add(TargetItemID);
	OnItemUpdated.Broadcast(TargetItemID);
	OnWeightChanged.Broadcast();
	OnInventoryChanged.Broadcast();

	return Result;
}

FInventoryOperationResult UInventoryComponent::TransferItem(const FGuid& InstanceID, UInventoryComponent* TargetInventory, FIntPoint TargetPosition, bool bRotated)
{
	FInventoryOperationResult Result;

	if (!TargetInventory)
	{
		Result.Message = FText::FromString(TEXT("目标背包无效"));
		Result.ResultCode = EInventoryResult::InvalidItem;
		return Result;
	}

	if (TargetInventory == this)
	{
		return MoveItem(InstanceID, TargetPosition, bRotated);
	}

	const int32 ItemIndex = Items.IndexOfByPredicate([&](const FInventoryItemInstance& Item)
	{
		return Item.InstanceID == InstanceID;
	});

	if (ItemIndex == INDEX_NONE)
	{
		Result.Message = FText::FromString(TEXT("物品不存在"));
		Result.ResultCode = EInventoryResult::InvalidItem;
		return Result;
	}

	FInventoryItemInstance TransferringItem = Items[ItemIndex];
	const FInventoryItemDefinition* Def = GetItemDefinition(TransferringItem.ItemID);

	if (Def && !TargetInventory->CanCarryWeight(Def->Weight * TransferringItem.Quantity))
	{
		Result.Message = FText::FromString(TEXT("目标背包超重"));
		Result.ResultCode = EInventoryResult::Overweight;
		return Result;
	}

	TransferringItem.Position = TargetPosition;
	TransferringItem.bRotated = bRotated;
	if (!TargetInventory->CanPlaceItem(TransferringItem, TargetPosition))
	{
		Result.Message = FText::FromString(TEXT("目标位置不可用"));
		Result.ResultCode = EInventoryResult::InvalidPosition;
		return Result;
	}

	Items.RemoveAt(ItemIndex);
	RebuildOccupancyMap();
	RecalculateWeight();
	OnItemRemoved.Broadcast(InstanceID);
	OnWeightChanged.Broadcast();
	OnInventoryChanged.Broadcast();

	TargetInventory->Items.Add(TransferringItem);
	TargetInventory->RebuildOccupancyMap();
	TargetInventory->RecalculateWeight();
	TargetInventory->OnItemAdded.Broadcast(TransferringItem);
	TargetInventory->OnWeightChanged.Broadcast();
	TargetInventory->OnInventoryChanged.Broadcast();

	Result.bSuccess = true;
	Result.AffectedItems.Add(InstanceID);
	return Result;
}

FInventoryOperationResult UInventoryComponent::SplitStack(const FGuid& InstanceID, int32 SplitAmount)
{
	FInventoryOperationResult Result;

	FInventoryItemInstance* Item = FindItem(InstanceID);
	if (!Item)
	{
		Result.Message = FText::FromString(TEXT("物品不存在"));
		Result.ResultCode = EInventoryResult::InvalidItem;
		return Result;
	}

	if (SplitAmount <= 0 || SplitAmount >= Item->Quantity)
	{
		Result.Message = FText::FromString(TEXT("拆分数量无效"));
		Result.ResultCode = EInventoryResult::InvalidItem;
		return Result;
	}

	const FInventoryItemDefinition* Def = GetItemDefinition(Item->ItemID);
	if (!Def || !Def->bStackable)
	{
		Result.Message = FText::FromString(TEXT("该物品不可堆叠，无法拆分"));
		Result.ResultCode = EInventoryResult::NotStackable;
		return Result;
	}

	if (!CanCarryWeight(Def->Weight * SplitAmount))
	{
		Result.Message = FText::FromString(TEXT("拆分后超重"));
		Result.ResultCode = EInventoryResult::Overweight;
		return Result;
	}

	FInventoryItemInstance TempItem;
	TempItem.ItemID = Item->ItemID;
	TempItem.Quantity = SplitAmount;
	TempItem.bRotated = Item->bRotated;

	FIntPoint NewPosition;
	if (!FindAvailablePosition(TempItem, NewPosition))
	{
		Result.Message = FText::FromString(TEXT("空间不足以放置拆分物品"));
		Result.ResultCode = EInventoryResult::NoSpace;
		return Result;
	}

	Item->Quantity -= SplitAmount;

	FInventoryItemInstance NewItem;
	NewItem.ItemID = Item->ItemID;
	NewItem.InstanceID = FGuid::NewGuid();
	NewItem.Quantity = SplitAmount;
	NewItem.bRotated = Item->bRotated;
	NewItem.Position = NewPosition;
	Items.Add(NewItem);

	RebuildOccupancyMap();
	RecalculateWeight();

	Result.bSuccess = true;
	Result.AffectedItems.Add(InstanceID);
	Result.AffectedItems.Add(NewItem.InstanceID);
	OnItemUpdated.Broadcast(InstanceID);
	OnItemAdded.Broadcast(NewItem);
	OnWeightChanged.Broadcast();
	OnInventoryChanged.Broadcast();

	return Result;
}

FInventoryOperationResult UInventoryComponent::AutoArrange()
{
	FInventoryOperationResult Result;

	if (Items.Num() == 0)
	{
		Result.bSuccess = true;
		return Result;
	}

	TArray<FInventoryItemInstance> SavedItems = Items;

	Items.Sort([this](const FInventoryItemInstance& A, const FInventoryItemInstance& B)
	{
		const FIntPoint SizeA = GetItemSize(A);
		const FIntPoint SizeB = GetItemSize(B);
		return (SizeA.X * SizeA.Y) > (SizeB.X * SizeB.Y);
	});

	for (FInventoryItemInstance& Item : Items)
	{
		Item.Position = FIntPoint::ZeroValue;
	}
	RebuildOccupancyMap();

	bool bAllPlaced = true;
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		FIntPoint NewPosition;
		if (!FindAvailablePosition(Items[i], NewPosition))
		{
			bAllPlaced = false;
			break;
		}
		Items[i].Position = NewPosition;
		RebuildOccupancyMap();
	}

	if (!bAllPlaced)
	{
		Items = MoveTemp(SavedItems);
		RebuildOccupancyMap();
		Result.Message = FText::FromString(TEXT("整理失败，空间不足"));
		Result.ResultCode = EInventoryResult::NoSpace;
		return Result;
	}

	Result.bSuccess = true;
	for (const FInventoryItemInstance& Item : Items)
	{
		Result.AffectedItems.Add(Item.InstanceID);
		OnItemUpdated.Broadcast(Item.InstanceID);
	}
	OnInventoryChanged.Broadcast();

	return Result;
}

FInventoryOperationResult UInventoryComponent::ResizeInventory(FIntPoint NewSize)
{
	FInventoryOperationResult Result;

	if (NewSize.X <= 0 || NewSize.Y <= 0)
	{
		Result.Message = FText::FromString(TEXT("无效尺寸"));
		Result.ResultCode = EInventoryResult::InvalidPosition;
		return Result;
	}

	if (NewSize == GridSize)
	{
		Result.bSuccess = true;
		return Result;
	}

	const FIntPoint OldSize = GridSize;
	const TArray<FGuid> OldMap = OccupancyMap;
	const TArray<FInventoryItemInstance> OldItems = Items;

	GridSize = NewSize;
	RebuildOccupancyMap();

	bool bAllInBounds = true;
	for (const FInventoryItemInstance& Item : Items)
	{
		const FIntPoint Size = GetItemSize(Item);
		if (Item.Position.X + Size.X > NewSize.X ||
			Item.Position.Y + Size.Y > NewSize.Y)
		{
			bAllInBounds = false;
			break;
		}
	}

	if (!bAllInBounds)
	{
		Items.Sort([this](const FInventoryItemInstance& A, const FInventoryItemInstance& B)
		{
			const FIntPoint SizeA = GetItemSize(A);
			const FIntPoint SizeB = GetItemSize(B);
			return (SizeA.X * SizeA.Y) > (SizeB.X * SizeB.Y);
		});

		for (FInventoryItemInstance& Item : Items)
		{
			Item.Position = FIntPoint::ZeroValue;
		}
		RebuildOccupancyMap();

		bool bAllPlaced = true;
		for (int32 i = 0; i < Items.Num(); ++i)
		{
			FIntPoint NewPosition;
			if (!FindAvailablePosition(Items[i], NewPosition))
			{
				bAllPlaced = false;
				break;
			}
			Items[i].Position = NewPosition;
			RebuildOccupancyMap();
		}

		if (!bAllPlaced)
		{
			Items = OldItems;
			GridSize = OldSize;
			OccupancyMap = OldMap;
			Result.Message = FText::FromString(TEXT("空间不足以容纳所有物品"));
		Result.ResultCode = EInventoryResult::NoSpace;
			return Result;
		}
	}

	Result.bSuccess = true;
	OnInventoryResized.Broadcast();
	OnInventoryChanged.Broadcast();

	return Result;
}

//=============================================================================
// 批量操作 API
//=============================================================================

FInventoryOperationResult UInventoryComponent::RemoveAll()
{
	FInventoryOperationResult Result;

	for (const FInventoryItemInstance& Item : Items)
	{
		Result.AffectedItems.Add(Item.InstanceID);
	}

	Items.Empty();
	RebuildOccupancyMap();
	RecalculateWeight();

	OnInventoryCleared.Broadcast();
	OnWeightChanged.Broadcast();
	OnInventoryChanged.Broadcast();

	Result.bSuccess = true;
	return Result;
}

FInventoryOperationResult UInventoryComponent::RemoveItemsByID(FName ItemRowID, int32 Count)
{
	FInventoryOperationResult Result;

	// 记录移除前的总数，用于计算实际移除数量
	const int32 BeforeCount = GetItemCount(ItemRowID);

	const int32 RemainingAfter = (Count <= 0) ? 0 : FMath::Max(0, BeforeCount - Count);

	if (Count <= 0 || RemainingAfter <= 0)
	{
		// 移除全部该类型物品
		for (int32 i = Items.Num() - 1; i >= 0; --i)
		{
			if (Items[i].ItemID == ItemRowID)
			{
				Result.AffectedItems.Add(Items[i].InstanceID);
				const FGuid RemovedID = Items[i].InstanceID;
				Items.RemoveAt(i);
				OnItemRemoved.Broadcast(RemovedID);
			}
		}
	}
	else
	{
		// 从后往前扣减数量
		int32 ToRemove = Count;
		for (int32 i = Items.Num() - 1; i >= 0 && ToRemove > 0; --i)
		{
			if (Items[i].ItemID != ItemRowID)
			{
				continue;
			}

			Result.AffectedItems.Add(Items[i].InstanceID);

			if (Items[i].Quantity <= ToRemove)
			{
				ToRemove -= Items[i].Quantity;
				const FGuid RemovedID = Items[i].InstanceID;
				Items.RemoveAt(i);
				OnItemRemoved.Broadcast(RemovedID);
			}
			else
			{
				Items[i].Quantity -= ToRemove;
				ToRemove = 0;
				OnItemUpdated.Broadcast(Items[i].InstanceID);
			}
		}
	}

	RebuildOccupancyMap();
	RecalculateWeight();

	OnWeightChanged.Broadcast();
	OnInventoryChanged.Broadcast();

	// 实际移除数量 = 移除前总数 - 移除后剩余总数
	Result.Amount = BeforeCount - GetItemCount(ItemRowID);
	Result.bSuccess = true;
	return Result;
}

FInventoryOperationResult UInventoryComponent::AddItemByID(FName ItemRowID, int32 Quantity)
{
	FInventoryOperationResult Result;

	if (Quantity <= 0)
	{
		Result.Message = FText::FromString(TEXT("数量必须大于 0"));
		Result.ResultCode = EInventoryResult::InvalidItem;
		return Result;
	}

	const FInventoryItemDefinition* Def = GetItemDefinition(ItemRowID);
	if (!Def)
	{
		Result.ResultCode = EInventoryResult::InvalidItem;
		Result.Message = FText::FromString(TEXT("无效物品ID"));
		return Result;
	}

	// 重量预算：根据剩余承重计算最多能加入的数量
	int32 Desired = Quantity;
	if (Def->Weight > 0.f)
	{
		const int32 MaxByWeight = FMath::FloorToInt((MaxWeight - CurrentWeight) / Def->Weight);
		Desired = FMath::Min(Desired, FMath::Max(0, MaxByWeight));
	}

	if (Desired <= 0)
	{
		Result.ResultCode = EInventoryResult::Overweight;
		Result.Message = FText::FromString(TEXT("超重，无法添加"));
		return Result;
	}

	int32 Remaining = Desired;

	// 阶段一：可堆叠且无实例数据 → 先填满已有同类型堆叠
	if (Def->bStackable && !Def->bUseInstanceData)
	{
		for (FInventoryItemInstance& ExistingItem : Items)
		{
			if (Remaining <= 0)
			{
				break;
			}
			if (ExistingItem.ItemID != ItemRowID || ExistingItem.Quantity >= Def->MaxStackSize)
			{
				continue;
			}

			const int32 Room = Def->MaxStackSize - ExistingItem.Quantity;
			const int32 ToAdd = FMath::Min(Room, Remaining);
			ExistingItem.Quantity += ToAdd;
			Remaining -= ToAdd;

			Result.AffectedItems.Add(ExistingItem.InstanceID);
			OnItemUpdated.Broadcast(ExistingItem.InstanceID);
		}
	}

	// 阶段二：剩余部分占新格
	while (Remaining > 0)
	{
		FInventoryItemInstance NewItem;
		NewItem.ItemID = ItemRowID;
		NewItem.InstanceID = FGuid::NewGuid();
		NewItem.bRotated = false;
		// 可堆叠物品按 MaxStackSize 分组；不可堆叠物品每次 1 个
		NewItem.Quantity = (Def->bStackable && !Def->bUseInstanceData)
			? FMath::Min(Remaining, Def->MaxStackSize)
			: 1;

		FIntPoint NewPosition;
		if (!FindAvailablePosition(NewItem, NewPosition))
		{
			// 空间不足，停止占新格
			break;
		}

		NewItem.Position = NewPosition;
		Items.Add(NewItem);

		Remaining -= NewItem.Quantity;
		Result.AffectedItems.Add(NewItem.InstanceID);
		OnItemAdded.Broadcast(NewItem);
	}

	// 实际加入数量
	Result.Amount = Desired - Remaining;

	RebuildOccupancyMap();
	RecalculateWeight();
	OnWeightChanged.Broadcast();
	OnInventoryChanged.Broadcast();

	if (Result.Amount > 0)
	{
		Result.bSuccess = true;
	}
	else
	{
		// 一个都没放进去
		Result.ResultCode = EInventoryResult::NoSpace;
		Result.Message = FText::FromString(TEXT("空间不足"));
	}

	return Result;
}

bool UInventoryComponent::SetQuickSlot(int32 SlotIndex, const FGuid& ItemID)
{
	if (SlotIndex < 0)
	{
		return false;
	}

	while (QuickSlots.Num() <= SlotIndex)
	{
		QuickSlots.Add(FQuickSlot());
	}

	if (ItemID.IsValid() && !FindItem(ItemID))
	{
		return false;
	}

	QuickSlots[SlotIndex].ItemInstanceID = ItemID;
	OnQuickSlotChanged.Broadcast();
	return true;
}

bool UInventoryComponent::ClearQuickSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= QuickSlots.Num())
	{
		return false;
	}

	QuickSlots[SlotIndex].ItemInstanceID = FGuid();
	OnQuickSlotChanged.Broadcast();
	return true;
}

bool UInventoryComponent::UseQuickSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= QuickSlots.Num())
	{
		return false;
	}

	const FGuid& ItemID = QuickSlots[SlotIndex].ItemInstanceID;
	if (!ItemID.IsValid())
	{
		return false;
	}

	const FInventoryItemInstance* Item = FindItem(ItemID);
	if (!Item)
	{
		return false;
	}

	UseItem(*Item);
	return true;
}

void UInventoryComponent::UseItem(const FInventoryItemInstance& Item)
{
	// 广播使用请求，游戏层监听并决定具体行为（消耗品、装备等）
	// 插件不实现玩法逻辑
	OnItemUseRequested.Broadcast(Item.InstanceID);
}

//=============================================================================
// 内部辅助函数
//=============================================================================

FIntPoint UInventoryComponent::GetItemSize(const FInventoryItemInstance& Item) const
{
	const FInventoryItemDefinition* Def = GetItemDefinition(Item.ItemID);
	if (!Def)
	{
		return FIntPoint(1, 1);
	}
	return Item.bRotated ? FIntPoint(Def->Size.Y, Def->Size.X) : Def->Size;
}

void UInventoryComponent::RebuildOccupancyMap()
{
	const int32 TotalCells = GridSize.X * GridSize.Y;
	OccupancyMap.SetNum(TotalCells);

	for (FGuid& Cell : OccupancyMap)
	{
		Cell = FGuid();
	}

	for (int32 ItemIndex = 0; ItemIndex < Items.Num(); ++ItemIndex)
	{
		const FInventoryItemInstance& Item = Items[ItemIndex];
		const FIntPoint Size = GetItemSize(Item);

		for (int32 y = 0; y < Size.Y; ++y)
		{
			for (int32 x = 0; x < Size.X; ++x)
			{
				const int32 CellX = Item.Position.X + x;
				const int32 CellY = Item.Position.Y + y;

				if (CellX >= 0 && CellX < GridSize.X && CellY >= 0 && CellY < GridSize.Y)
				{
					OccupancyMap[CellY * GridSize.X + CellX] = Item.InstanceID;
				}
			}
		}
	}
}

void UInventoryComponent::RecalculateWeight()
{
	CurrentWeight = 0.f;
	for (const FInventoryItemInstance& Item : Items)
	{
		const FInventoryItemDefinition* Def = GetItemDefinition(Item.ItemID);
		if (Def)
		{
			CurrentWeight += Def->Weight * static_cast<float>(Item.Quantity);
		}
	}
}

//=============================================================================
// 右键菜单 API
//=============================================================================

TArray<FGameplayTag> UInventoryComponent::GetAvailableActions(const FGuid& InstanceID) const
{
	TArray<FGameplayTag> Result;

	// const FindItem 不可用，通过 const_cast 调用已有的非 const 版本
	FInventoryItemInstance* Item = const_cast<UInventoryComponent*>(this)->FindItem(InstanceID);
	if (!Item)
	{
		return Result;
	}

	const FInventoryItemDefinition* Def = GetItemDefinition(Item->ItemID);
	if (!Def)
	{
		return Result;
	}

	return Def->EnabledActions.GetGameplayTagArray();
}

bool UInventoryComponent::ExecuteAction(FGameplayTag ActionTag, const FGuid& InstanceID)
{
	FInventoryItemInstance* Item = FindItem(InstanceID);
	if (!Item)
	{
		return false;
	}

	if (ActionTag == TAG_Inventory_Action_使用)
	{
		UseItem(*Item);
		return true;
	}
	if (ActionTag == TAG_Inventory_Action_丢弃)
	{
		DropItem(InstanceID);
		return true;
	}
	if (ActionTag == TAG_Inventory_Action_旋转)
	{
		RotateItem(InstanceID);
		return true;
	}
	if (ActionTag == TAG_Inventory_Action_拆分)
	{
		if (Item->Quantity > 1)
		{
			SplitStack(InstanceID, Item->Quantity / 2);
		}
		return true;
	}

	// 非内置标签 → 转给 Blueprint 扩展处理
	OnCustomAction(ActionTag, InstanceID);
	return true;
}

//=============================================================================
// 存档 API
//=============================================================================

FInventorySaveData UInventoryComponent::SaveToData() const
{
	FInventorySaveData Data;
	Data.Items = Items;
	Data.QuickSlots = QuickSlots;
	Data.GridSize = GridSize;
	Data.MaxWeight = MaxWeight;
	return Data;
}

void UInventoryComponent::LoadFromData(const FInventorySaveData& Data)
{
	// 静默替换数据（不逐项广播，避免 N 次 RefreshAllItems）
	Items = Data.Items;
	QuickSlots = Data.QuickSlots;
	GridSize = Data.GridSize;
	MaxWeight = Data.MaxWeight;

	// 重建派生数据
	RebuildOccupancyMap();
	RecalculateWeight();
	DefinitionCache.Empty();

	// 批量通知 UI 全量刷新（Grid 绑了 OnInventoryCleared → RefreshAllItems 一次完成）
	OnInventoryCleared.Broadcast();
	OnInventoryResized.Broadcast();
	OnWeightChanged.Broadcast();
	OnQuickSlotChanged.Broadcast();
	OnInventoryChanged.Broadcast();
}
