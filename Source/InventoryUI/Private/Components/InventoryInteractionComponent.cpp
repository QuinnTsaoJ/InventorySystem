// 背包交互组件实现
// InventorySystem Plugin

#include "Components/InventoryInteractionComponent.h"

#include "EnhancedInputComponent.h"
#include "Widgets/InventoryPanelWidget.h"
#include "Widgets/QuickBarWidget.h"
#include "InventoryComponent.h"
#include "Interfaces/InventoryProviderInterface.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"

UInventoryInteractionComponent::UInventoryInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	static ConstructorHelpers::FClassFinder<UInventoryPanelWidget> UInventoryPanelFinder(
		TEXT("/InventorySystem/UI/Widgets/Inventory/WBP_InventoryPanel"));
	if (UInventoryPanelFinder.Succeeded())
	{
		InventoryWidgetClass = UInventoryPanelFinder.Class;
	}
	
	static ConstructorHelpers::FClassFinder<UQuickBarWidget> UQuickBarFinder(
		TEXT("/InventorySystem/UI/Widgets/QuickBar/WBP_QuickBar"));
	if (UQuickBarFinder.Succeeded())
	{
		QuickBarWidgetClass = UQuickBarFinder.Class;
	}
}

void UInventoryInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	// 创建背包面板，仅一次，初始隐藏
	if (InventoryWidgetClass)
	{
		InventoryWidget = CreateWidget<UInventoryPanelWidget>(PC, InventoryWidgetClass);
		if (InventoryWidget)
		{
			InventoryWidget->AddToViewport(9);
			InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 创建快捷栏，独立于背包面板，始终可见
	if (QuickBarWidgetClass)
	{
		QuickBarWidget = CreateWidget<UQuickBarWidget>(PC, QuickBarWidgetClass);
		if (QuickBarWidget)
		{
			QuickBarWidget->AddToViewport(10);

			if (UInventoryComponent* Inv = ResolvePlayerInventory())
			{
				QuickBarWidget->InitializeQuickBar(Inv);
			}
		}
	}

	
	SetupInputBindings();
}

void UInventoryInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveInputBindings();

	if (InventoryWidget)
	{
		InventoryWidget->RemoveFromParent();
		InventoryWidget = nullptr;
	}

	if (QuickBarWidget)
	{
		QuickBarWidget->RemoveFromParent();
		QuickBarWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

//=============================================================================
// 公共 API
//=============================================================================

void UInventoryInteractionComponent::OpenPlayerInventory()
{
	UInventoryComponent* Resolved = ResolvePlayerInventory();
	if (!Resolved)
	{
		return;
	}

	OpenInventorySession(Resolved, nullptr);
}

void UInventoryInteractionComponent::OpenExternalInventory(UInventoryComponent* InExternalInventory)
{
	if (!InExternalInventory)
	{
		return;
	}

	UInventoryComponent* Resolved = ResolvePlayerInventory();
	if (!Resolved)
	{
		return;
	}

	OpenInventorySession(Resolved, InExternalInventory);
}

void UInventoryInteractionComponent::CloseInventory()
{
	if (!InventoryWidget||!InventoryWidget->IsVisible())
	{
		return;
	}
	
	InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	ExternalInventory.Reset();
	SetInputModeForInventory(false);
	OnInventoryClosed();
}

void UInventoryInteractionComponent::ToggleInventory()
{
	if (InventoryWidget->IsVisible())
	{
		CloseInventory();
	}
	else
	{
		OpenPlayerInventory();
	}
}

//=============================================================================
// 内部实现
//=============================================================================

void UInventoryInteractionComponent::OpenInventorySession(UInventoryComponent* InPlayerInventory, UInventoryComponent* InExternalInventory)
{
	if (!InventoryWidget || !InPlayerInventory)
	{
		return;
	}

	PlayerInventory = InPlayerInventory;
	ExternalInventory = InExternalInventory;

	// 初始化独立快捷栏（绑定玩家背包）
	if (QuickBarWidget)
	{
		QuickBarWidget->InitializeQuickBar(InPlayerInventory);
	}

	// 初始化面板：单/双网格模式
	if (InExternalInventory)
	{
		InventoryWidget->InitializeDualInventory(InPlayerInventory, InExternalInventory);
		OnExternalInventoryOpened(InExternalInventory);
	}
	else
	{
		InventoryWidget->InitializeInventory(InPlayerInventory);
	}

	InventoryWidget->SetVisibility(ESlateVisibility::Visible);
	
	SetInputModeForInventory(true);
	InventoryWidget->SetFocus();

	if (APlayerController* OwnPC = Cast<APlayerController>(GetOwner()))
	{
		int32 VpX, VpY;
		OwnPC->GetViewportSize(VpX, VpY);
		OwnPC->SetMouseLocation(VpX / 2, VpY / 2);
	}

	OnInventoryOpened();
}

UInventoryComponent* UInventoryInteractionComponent::ResolvePlayerInventory() const
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		return nullptr;
	}

	APawn* Pawn = PC->GetPawn();
	if (!Pawn)
	{
		return nullptr;
	}

	// 通过接口解析，不硬转换具体角色类型
	if (Pawn->Implements<UInventoryProviderInterface>())
	{
		return IInventoryProviderInterface::Execute_GetInventoryComponent(Pawn);
	}

	return nullptr;
}

void UInventoryInteractionComponent::SetInputModeForInventory(bool bOpen)
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		return;
	}

	if (bOpen)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
	else
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}

//=============================================================================
// 输入绑定
//=============================================================================

void UInventoryInteractionComponent::SetupInputBindings()
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		return;
	}
	
	UInputComponent* InputComp = PC->InputComponent;
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComp);
	if (!EnhancedInputComponent)
	{
		return;
	}

	CachedInputComponent = InputComp;

	EnhancedInputComponent->BindAction(IA_ToggleInventory, ETriggerEvent::Triggered, this, &UInventoryInteractionComponent::OnInputToggleInventory);
	EnhancedInputComponent->BindAction(IA_CloseInventory, ETriggerEvent::Triggered, this, &UInventoryInteractionComponent::OnInputCloseInventory);
	EnhancedInputComponent->BindAction(IA_RotateItem, ETriggerEvent::Triggered, this, &UInventoryInteractionComponent::OnInputRotateItem);

	EnhancedInputComponent->BindAction(IA_QuickSlot1, ETriggerEvent::Triggered, this, &UInventoryInteractionComponent::OnInputQuickSlot1);
	EnhancedInputComponent->BindAction(IA_QuickSlot2, ETriggerEvent::Triggered, this, &UInventoryInteractionComponent::OnInputQuickSlot2);
	EnhancedInputComponent->BindAction(IA_QuickSlot3, ETriggerEvent::Triggered, this, &UInventoryInteractionComponent::OnInputQuickSlot3);
	EnhancedInputComponent->BindAction(IA_QuickSlot4, ETriggerEvent::Triggered, this, &UInventoryInteractionComponent::OnInputQuickSlot4);
	EnhancedInputComponent->BindAction(IA_QuickSlot5, ETriggerEvent::Triggered, this, &UInventoryInteractionComponent::OnInputQuickSlot5);
	EnhancedInputComponent->BindAction(IA_QuickSlot6, ETriggerEvent::Triggered, this, &UInventoryInteractionComponent::OnInputQuickSlot6);
}

void UInventoryInteractionComponent::RemoveInputBindings()
{
	// InputComponent 由 PlayerController 管理生命周期，
	// 这里仅清除缓存引用即可
	CachedInputComponent = nullptr;
}

void UInventoryInteractionComponent::OnInputToggleInventory()
{
	ToggleInventory();
}

void UInventoryInteractionComponent::OnInputCloseInventory()
{
	if (InventoryWidget->IsVisible())
	{
		CloseInventory();
	}
}

void UInventoryInteractionComponent::OnInputRotateItem()
{
	// 旋转由拖拽逻辑内部处理，这里仅做键盘事件转发占位
}

void UInventoryInteractionComponent::HandleQuickSlotInput(int32 SlotIndex)
{
	if (!PlayerInventory.IsValid())
	{
		return;
	}

	PlayerInventory->UseQuickSlot(SlotIndex);
	OnQuickSlotUsed(SlotIndex);
}
