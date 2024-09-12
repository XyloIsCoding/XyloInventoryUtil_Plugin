// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryComponent.h"

#include "Inventory/XIUInventoryFunctionLibrary.h"
#include "Inventory/XIUItemStack.h"
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

bool FXIUInventoryList::CanManipulateInventory() const
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

TArray<UXIUItemStack*> FXIUInventoryList::AddItem(UXIUItem* Item, int32 Count)
{
	checkf(Count > 0, TEXT("Cannot add negative amounts"));
	check(Item != nullptr);
	check(CanManipulateInventory());

	UE_LOG(LogTemp, Warning, TEXT("--AddItem--"))
	int RemainingCount = Count;

	// add count to existing stacks
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (Slot.Stack && Slot.Stack->GetItem() == Item)
		{
			RemainingCount -= Slot.Stack->ModifyCount(RemainingCount);
			UE_LOG(LogTemp, Warning, TEXT("try add to existing stack (+ %i) : Remaining %i, Count %i"), Count, RemainingCount, Slot.Stack->GetCount())
			if (RemainingCount <= 0) break;
		}
	}
	
	// add new stacks with remaining count
	TArray<UXIUItemStack*> Results;
	while (RemainingCount > 0)
	{
		UXIUItemStack* Result = UXIUInventoryFunctionLibrary::MakeItemStackFromItem(OwnerComponent, Item);  //@TODO: Using the actor instead of component as the outer due to UE-127172
		const int NewRemainingCount = RemainingCount - Result->SetCount(RemainingCount);

		// not able to add count to stack (max count = 0)
		if (NewRemainingCount == RemainingCount)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot add stacks with max count = 0"))
			break;
		}
		
		if (AddItemStackInEmptySlot(Result, false))
		{
			RemainingCount = NewRemainingCount;
			Results.Add(Result);
		}
		// unable to add stack to inventory (probably full)
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Inventory is full, cannot add remaining stacks"))
			break;
		}
	}
	return Results;
}

bool FXIUInventoryList::AddItemStack(UXIUItemStack* ItemStack, bool bUpdateOwningInventory)
{
	check(ItemStack != nullptr);
	check(CanManipulateInventory());

	UE_LOG(LogTemp, Warning, TEXT("--AddItem--"))

	bool bAddedAny = false;
	
	// add count to existing stacks
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (Slot.Stack && Slot.Stack->GetItem() == ItemStack->GetItem()) //TODO: check that stacks are equal, not stacks' item
		{
			// add count to slot stack
			int CountToAdd = ItemStack->GetCount();
			int AddedCount = Slot.Stack->ModifyCount(CountToAdd);

			if (AddedCount > 0)
			{
				bAddedAny = true;
				// remove count from original stack
				ItemStack->ModifyCount(-AddedCount);
				UE_LOG(LogTemp, Warning, TEXT("try add to existing stack (+ %i) : Remaining %i, Count %i"), CountToAdd, (CountToAdd - AddedCount), Slot.Stack->GetCount())
			}
			
			// no more count to add (ItemStack->GetCount() == 0)
			if (CountToAdd - AddedCount <= 0)
			{
				ItemStack->SetOwningInventoryComponent(nullptr);
				break;
			}
		}
	}
	
	// try to add stacks with remaining count
	if (ItemStack->GetCount() > 0 && AddItemStackInEmptySlot(ItemStack, bUpdateOwningInventory))
	{
		bAddedAny = true;
	}
	return bAddedAny;
}

bool FXIUInventoryList::AddItemStackInEmptySlot(UXIUItemStack* ItemStack, bool bUpdateOwningInventory)
{
	check(ItemStack != nullptr);
	check(CanManipulateInventory());

	for (FXIUInventorySlot& Slot : Entries)
	{
		// if slot is empty I can set stack
		if (Slot.Stack == nullptr)
		{
			ItemStack->SetOwningInventoryComponent(OwnerComponent);
			Slot.SetItemStack(ItemStack);
			MarkItemDirty(Slot);
			
			UE_LOG(LogTemp, Warning, TEXT("added new stack"))
			return true;
		}
	}
	return false;
}

int FXIUInventoryList::RemoveCountFromItemStack(UXIUItemStack* ItemStack, int32 Count)
{
	checkf(Count > 0, TEXT("Cannot remove negative amounts"));
	check(ItemStack != nullptr);
	check(CanManipulateInventory());
	
	for (auto SlotIt = Entries.CreateIterator(); SlotIt; ++SlotIt)
	{
		FXIUInventorySlot& Slot = *SlotIt;
		if (Slot.Stack == ItemStack)
		{
			Count += Slot.Stack->ModifyCount(-Count); // add to count the delta count which is negative, so im actually subtracting the count removed
			if (Slot.Stack->GetCount() <= 0)
			{
				ClearSlot(SlotIt.GetIndex());
			}
		}
	}
	return Count;
}

int FXIUInventoryList::ConsumeItem(UXIUItem* Item, int32 Count)
{
	checkf(Count > 0, TEXT("Cannot consume negative amounts"));
	check(Item != nullptr);
	check(CanManipulateInventory());
	
	for (auto SlotIt = Entries.CreateIterator(); SlotIt; ++SlotIt)
	{
		FXIUInventorySlot& Slot = *SlotIt;
		if (Slot.Stack && Slot.Stack->GetItem() == Item)
		{
			Count += Slot.Stack->ModifyCount(-Count); // add to count the delta count which is negative, so im actually subtracting the count removed
			if (Slot.Stack->GetCount() <= 0)
			{
				ClearSlot(SlotIt.GetIndex());
			}
		}
	}
	return Count;
}

void FXIUInventoryList::ClearSlot(int SlotIndex)
{
	check(CanManipulateInventory());
	
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (Slot.Index == SlotIndex)
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
	bReplicateUsingRegisteredSubObjectList = true;
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
    		AddItem(Item, 2);
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






bool UXIUInventoryComponent::CanAddItem(TSubclassOf<UXIUItem> Item, int32 Count)
{
	//@TODO: Add support for stack limit / uniqueness checks / etc...
	return true;
}

TArray<UXIUItemStack*> UXIUInventoryComponent::AddItem(UXIUItem* Item, const int32 Count)
{
	TArray<UXIUItemStack*> Result;
	if (Item != nullptr)
	{
		Result = Inventory.AddItem(Item, Count);
	}
	return Result;
}

void UXIUInventoryComponent::AddItemStack(UXIUItemStack* ItemStack)
{
	if (ItemStack != nullptr)
	{
		Inventory.AddItemStack(ItemStack);
	}
}

bool UXIUInventoryComponent::ConsumeItem(UXIUItem* Item, int32 Count)
{
	if (Item != nullptr)
	{
		if (Inventory.ConsumeItem(Item, Count) == 0)
		{
			return true;
		}
	}
	return false;
}


TArray<UXIUItemStack*> UXIUInventoryComponent::GetAllItems() const
{
	return Inventory.GetAllItems();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * SubObjects Replication
 */

bool UXIUInventoryComponent::RegisterReplicatedObject(UObject* ObjectToRegister)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && IsValid(ObjectToRegister))
	{
		ReplicatedObjects.AddUnique(ObjectToRegister);
		AddReplicatedSubObject(ObjectToRegister);
		return true;
	}
	return false;
}

bool UXIUInventoryComponent::UnregisterReplicatedObject(UObject* ObjectToUnregister)
{
	if (IsUsingRegisteredSubObjectList() && IsValid(ObjectToUnregister))
	{
		ReplicatedObjects.Remove(ObjectToUnregister);
		RemoveReplicatedSubObject(ObjectToUnregister);
		return true;
	}
	return false;
}