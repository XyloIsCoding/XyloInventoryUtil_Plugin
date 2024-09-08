// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryComponent.h"

#include "Engine/ActorChannel.h"
#include "Inventory/XIUInventoryFunctionLibrary.h"
#include "Inventory/XIUItemStack.h"
#include "Inventory/Fragment/XIUCountFragment.h"
#include "Net/UnrealNetwork.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * FXIUInventorySlot Interface
 */


FString FXIUInventorySlot::GetDebugString() const
{
	TSubclassOf<UXIUItem> Item;
	if (Stack != nullptr)
	{
		Item = Stack->GetItem()->GetClass();
	}

	return FString::Printf(TEXT("%s (%d x %s)"), *GetNameSafe(Stack), Stack->GetCount(), *GetNameSafe(Item));
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Stack */

void FXIUInventorySlot::Clear()
{
	Stack = nullptr;
}

bool FXIUInventorySlot::SetItemStack(TObjectPtr<UXIUItemStack> NewStack)
{
	if (bLocked) return false;
	
	if (!Filter || !NewStack || NewStack->GetItem() && NewStack->GetItem()->IsA(Filter))
	{
		Stack = NewStack;
		return true;
	}
	return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Filter */

void FXIUInventorySlot::SetFilter(TSubclassOf<UXIUItem> NewFilter)
{
	Filter = NewFilter;
}

/*--------------------------------------------------------------------------------------------------------------------*/
	
/*--------------------------------------------------------------------------------------------------------------------*/
/* Locked */

void FXIUInventorySlot::SetLocked(bool NewLock)
{
	bLocked = NewLock;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * FXIUInventory Interface
 */


/*--------------------------------------------------------------------------------------------------------------------*/
/* FFastArraySerializer contract */

void FXIUInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
}

void FXIUInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
}

void FXIUInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
}

/*--------------------------------------------------------------------------------------------------------------------*/

void FXIUInventoryList::BroadcastChangeMessage(FXIUInventorySlot& Entry, int32 OldCount, int32 NewCount)
{
}

bool FXIUInventoryList::CanManipulateInventory()
{
	if (!OwnerComponent) return false;
	AActor* OwningActor = OwnerComponent->GetOwner();
	if (!OwningActor->HasAuthority()) return false;
	return true;
}

void FXIUInventoryList::InitInventory(int Size)
{
	check(CanManipulateInventory());
	
	Entries.Empty();
	Entries.Reserve(Size);
	for (int i = 0; i < Size ; i++)
	{
		Entries.Add(FXIUInventorySlot(i));
	}
	MarkArrayDirty();
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Slots Management */

TArray<UXIUItemStack*> FXIUInventoryList::GetAllItems() const
{
	TArray<UXIUItemStack*> Results;
	Results.Reserve(Entries.Num());
	for (const FXIUInventorySlot& Slot : Entries)
	{
		if (Slot.Stack != nullptr) //@TODO: Would prefer to not deal with this here and hide it further?
		{
			Results.Add(Slot.Stack);
		}
	}
	return Results;
}

UXIUItemStack* FXIUInventoryList::AddItem(UXIUItem* Item, int32 StackCount)
{
	UXIUItemStack* Result = nullptr;

	check(Item != nullptr);
	check(CanManipulateInventory());

	UE_LOG(LogTemp, Warning, TEXT("--AddItem--"))
	int RemainingCount = StackCount;
	
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (Slot.GetItemStack() && Slot.GetItemStack()->GetItem() == Item)
		{
			RemainingCount -= Slot.GetItemStack()->AddCount(StackCount);
			UE_LOG(LogTemp, Warning, TEXT("try add to existing stack (+ %i) : Remaining %i, Count %i"), StackCount, RemainingCount, Slot.GetItemStack()->GetCount())
			if (RemainingCount <= 0) break;
		}
	}

	// TODO: RemainingCount might be > then MaxCount. in that case i need to add multiple stacks (the function should return array of itemStacks to add all of them as subobjects)
	if (RemainingCount > 0)
	{
		Result = UXIUInventoryFunctionLibrary::MakeItemStackFromItem(OwnerComponent->GetOwner(), Item);  //@TODO: Using the actor instead of component as the outer due to UE-127172
		Result->SetCount(RemainingCount);
		
		for (FXIUInventorySlot& Slot : Entries)
		{
			if (Slot.GetItemStack() == nullptr)
			{
				Slot.SetItemStack(Result);
				UE_LOG(LogTemp, Warning, TEXT("added new stack"))
				MarkItemDirty(Slot);
				break;
			}
		}
	}

	return Result;
}

void FXIUInventoryList::AddItemStack(UXIUItemStack* Stack)
{
	check(Stack != nullptr);
	check(CanManipulateInventory());
	
	unimplemented();
}

void FXIUInventoryList::RemoveCountFromItemStack(UXIUItemStack* Stack, int32 StackCount)
{
	check(Stack != nullptr);
	check(CanManipulateInventory());
	
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (Slot.Stack == Stack)
		{
			int OldCount = Slot.Stack->GetCount();
			Slot.Stack->SetCount(OldCount - 1);
		}
	}
}

void FXIUInventoryList::ClearSlot(int SlotIndex)
{
	check(CanManipulateInventory());
	
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (Slot.GetIndex() == SlotIndex)
		{
			Slot.Clear();
		}
	}
}

/*--------------------------------------------------------------------------------------------------------------------*/



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


UXIUInventoryComponent::UXIUInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Inventory(this)
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UActorComponent Interface
 */

void UXIUInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		Inventory.InitInventory(FMath::Max(InventorySize, DefaultItems.Num()));
		AddDefaultItems();
	}
}

void UXIUInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UXIUInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Inventory);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Inventory
 */

void UXIUInventoryComponent::AddDefaultItems()
{
	ServerAddDefaultItems();
}

void UXIUInventoryComponent::ServerAddDefaultItems_Implementation()
{
	if (GetOwner() && GetOwner()->HasAuthority())
    {
    	for (TObjectPtr<UXIUItem> Item : DefaultItems)
    	{
    		AddItemDefinition(Item, 2);
    	}
    }
}

void UXIUInventoryComponent::PrintItems()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Orange, FString::Printf(TEXT("Inventory Size: %i"), Inventory.Entries.Num()));
	for (UXIUItemStack* Stack : Inventory.GetAllItems())
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Orange, FString::Printf(TEXT("Item: %s  ;  Count %i ; Fragments: %i"), *Stack->GetItem()->Name, Stack->GetCount(), Stack->GetAllFragments().Num()));
	}
}






bool UXIUInventoryComponent::CanAddItemDefinition(TSubclassOf<UXIUItem> ItemDef, int32 StackCount)
{
	//@TODO: Add support for stack limit / uniqueness checks / etc...
	return true;
}

UXIUItemStack* UXIUInventoryComponent::AddItemDefinition(UXIUItem* ItemDef, int32 StackCount)
{
	UXIUItemStack* Result = nullptr;
	if (ItemDef != nullptr)
	{
		Result = Inventory.AddItem(ItemDef, StackCount);
		
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Result)
		{
			RegisterReplicatedObject(Result);
			for (UXIUItemFragment* Fragment : Result->GetAllFragments())
			{
				RegisterReplicatedObject(Fragment);
			}
		}
	}
	return Result;
}

void UXIUInventoryComponent::AddItemInstance(UXIUItemStack* ItemInstance)
{
	Inventory.AddItemStack(ItemInstance);
	
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
	{
		RegisterReplicatedObject(ItemInstance);
		for (UXIUItemFragment* Fragment : ItemInstance->GetAllFragments())
		{
			RegisterReplicatedObject(Fragment);
		}
	}
}

void UXIUInventoryComponent::RemoveItemInstance(UXIUItemStack* ItemInstance)
{
	Inventory.RemoveCountFromItemStack(ItemInstance, 1);

	if (ItemInstance && IsUsingRegisteredSubObjectList())
	{
		UnregisterReplicatedObject(ItemInstance);
		for (UXIUItemFragment* Fragment : ItemInstance->GetAllFragments())
		{
			UnregisterReplicatedObject(Fragment);
		}
	}
}





TArray<UXIUItemStack*> UXIUInventoryComponent::GetAllItems() const
{
	return Inventory.GetAllItems();
}

UXIUItemStack* UXIUInventoryComponent::FindFirstItemStackByDefinition(TSubclassOf<UXIUItem> ItemDef) const
{
	for (const FXIUInventorySlot& Entry : Inventory.Entries)
	{
		UXIUItemStack* Instance = Entry.Stack;

		if (IsValid(Instance))
		{
			if (Instance->GetItem()->GetClass() == ItemDef)
			{
				return Instance;
			}
		}
	}

	return nullptr;
}

int32 UXIUInventoryComponent::GetTotalItemCountByDefinition(TSubclassOf<UXIUItem> ItemDef) const
{
	int32 TotalCount = 0;
	for (const FXIUInventorySlot& Entry : Inventory.Entries)
	{
		UXIUItemStack* Instance = Entry.Stack;

		if (IsValid(Instance))
		{
			if (Instance->GetItem()->GetClass() == ItemDef)
			{
				++TotalCount;
			}
		}
	}

	return TotalCount;
}

bool UXIUInventoryComponent::ConsumeItemsByDefinition(TSubclassOf<UXIUItem> ItemDef, int32 NumToConsume)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority())
	{
		return false;
	}

	//@TODO: N squared right now as there's no acceleration structure
	int32 TotalConsumed = 0;
	while (TotalConsumed < NumToConsume)
	{
		if (UXIUItemStack* Instance = UXIUInventoryComponent::FindFirstItemStackByDefinition(ItemDef))
		{
			Inventory.RemoveCountFromItemStack(Instance, 1);
			++TotalConsumed;
		}
		else
		{
			return false;
		}
	}

	return TotalConsumed == NumToConsume;
}