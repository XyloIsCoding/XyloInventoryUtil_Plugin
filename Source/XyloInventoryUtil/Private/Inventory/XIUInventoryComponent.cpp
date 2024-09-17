// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryComponent.h"

#include "Inventory/XIUInventoryFunctionLibrary.h"
#include "Inventory/XIUItemDefinition.h"
#include "Inventory/XIUItemStack.h"
#include "Net/UnrealNetwork.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * FXIUInventorySlot Interface
 */


FString FXIUInventorySlot::GetDebugString() const
{
	TSubclassOf<UXIUItem> Item;
	if (Stack != nullptr)
	{
		Item = Stack->GetItemDefinition()->GetClass();
	}

	return FString::Printf(TEXT("%s (%d x %s)"), *GetNameSafe(Stack), Stack->GetCount(), *GetNameSafe(Item));
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Stack */

bool FXIUInventorySlot::Clear(TObjectPtr<UXIUItemStack>& OldStack)
{
	if (Stack)
	{
		OldStack = Stack;
		Stack = nullptr;
		return true;
	}
	return false;
}

bool FXIUInventorySlot::SetItemStack(TObjectPtr<UXIUItemStack> NewStack, TObjectPtr<UXIUItemStack>& OldStack)
{
	if (bLocked) return false;
	
	if (MatchesFilter(NewStack))
	{
		OldStack = Stack;
		Stack = NewStack;
		return true;
	}
	return false;
}

bool FXIUInventorySlot::IsEmpty() const
{
	return Stack == nullptr || Stack->IsEmpty(); 
}

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Filter */

void FXIUInventorySlot::SetFilter(TSubclassOf<UXIUItem> NewFilter)
{
	Filter = NewFilter;
}

bool FXIUInventorySlot::MatchesFilter(const TObjectPtr<UXIUItemStack> ItemStack) const
{
	return !Filter || !ItemStack || (ItemStack->GetItemDefinition() && ItemStack->GetItemDefinition()->IsA(Filter));
}

/*--------------------------------------------------------------------------------------------------------------------*/
	
/*--------------------------------------------------------------------------------------------------------------------*/
/* Locked */

void FXIUInventorySlot::SetLocked(bool NewLock)
{
	bLocked = NewLock;
}

/*--------------------------------------------------------------------------------------------------------------------*/







////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
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
		FXIUInventorySlot& NewSlot = Entries.AddDefaulted_GetRef();
		NewSlot.Index = i;
		MarkItemDirty(NewSlot);
	}
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Slots Management */

const TArray<FXIUInventorySlot>& FXIUInventoryList::GetInventory() const
{
	return Entries;
}

UXIUItemStack* FXIUInventoryList::GetStackAtSlot(int SlotIndex)
{
	checkf(SlotIndex <= Entries.Num(), TEXT("Cannot get non existent slots"));
	
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (Slot.GetIndex() == SlotIndex)
		{
			return Slot.GetItemStack();
		}
	}
	return nullptr; 
}

int FXIUInventoryList::AddItem(UXIUItemDefinition* ItemDefinition, int32 Count, TArray<UXIUItemStack*> AddedStacks)
{
	checkf(Count > 0, TEXT("Cannot add negative amounts"));
	check(ItemDefinition != nullptr);
	check(CanManipulateInventory());

	UE_LOG(LogTemp, Warning, TEXT("--AddItem--"))
	int RemainingCount = Count;

	// add count to existing stacks
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (Slot.GetItemStack() && Slot.GetItemStack()->GetItemDefinition() == ItemDefinition)
		{
			RemainingCount -= Slot.GetItemStack()->ModifyCount(RemainingCount);
			UE_LOG(LogTemp, Warning, TEXT("try add to existing stack (+ %i) : Remaining %i, Count %i"), Count, RemainingCount, Slot.GetItemStack()->GetCount())
			if (RemainingCount <= 0) break;
		}
	}
	
	// add new stacks with remaining count
	while (RemainingCount > 0)
	{
		UXIUItemStack* Result = UXIUInventoryFunctionLibrary::MakeItemStackFromItem(OwnerComponent, ItemDefinition);  //@TODO: Using the actor instead of component as the outer due to UE-127172
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
			AddedStacks.Add(Result);
		}
		// unable to add stack to inventory (probably full)
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Inventory is full, cannot add remaining stacks"))
			break;
		}
	}
	return Count - RemainingCount;
}

int FXIUInventoryList::AddItemStack(UXIUItemStack* ItemStack, bool bUpdateOwningInventory)
{
	check(ItemStack != nullptr);
	check(CanManipulateInventory());

	UE_LOG(LogTemp, Warning, TEXT("--AddItemStack--"))

	int TotalAddedCount = 0;
	
	// add count to existing stacks
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (Slot.GetItemStack() && ItemStack->Matches(Slot.GetItemStack())) 
		{
			// add count to slot stack
			int CountToAdd = ItemStack->GetCount();
			int AddedCount = Slot.GetItemStack()->ModifyCount(CountToAdd);

			if (AddedCount > 0)
			{
				TotalAddedCount += AddedCount;
				// remove count from original stack
				ItemStack->ModifyCount(-AddedCount);
				UE_LOG(LogTemp, Warning, TEXT("try add to existing stack (+ %i) : Remaining %i, Count %i"), CountToAdd, (CountToAdd - AddedCount), Slot.GetItemStack()->GetCount())
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
	const int RemainingCount = ItemStack->GetCount();
	if (RemainingCount > 0 && AddItemStackInEmptySlot(ItemStack, bUpdateOwningInventory))
	{
		TotalAddedCount += RemainingCount;
	}
	return TotalAddedCount;
}

bool FXIUInventoryList::AddItemStackInEmptySlot(UXIUItemStack* ItemStack, bool bUpdateOwningInventory)
{
	check(ItemStack != nullptr);
	check(CanManipulateInventory());

	for (FXIUInventorySlot& Slot : Entries)
	{
		// if slot is empty I can set stack
		if (Slot.IsEmpty())
		{
			TObjectPtr<UXIUItemStack> OldStack;
			// Check if the stack was actually set (might be impossible due to filters or locked stacks)
			if (Slot.SetItemStack(ItemStack, OldStack))
			{
				if (bUpdateOwningInventory) ItemStack->SetOwningInventoryComponent(OwnerComponent);
				MarkItemDirty(Slot);
			
				UE_LOG(LogTemp, Warning, TEXT("added new stack"))
				return true;
			}
		}
	}
	return false;
}

bool FXIUInventoryList::RemoveItemStack(UXIUItemStack* ItemStack)
{
	check(ItemStack != nullptr);
	check(CanManipulateInventory());
	
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (Slot.GetItemStack() == ItemStack)
		{
			TObjectPtr<UXIUItemStack> RemovedStack;
			return Slot.Clear(RemovedStack);
		}
	}
	return false;
}

int FXIUInventoryList::RemoveCountFromItemStack(UXIUItemStack* ItemStack, int32 Count)
{
	checkf(Count > 0, TEXT("Cannot remove negative amounts"));
	check(ItemStack != nullptr);
	check(CanManipulateInventory());
	
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (Slot.GetItemStack() == ItemStack)
		{
			Count += Slot.GetItemStack()->ModifyCount(-Count); // add to count the delta count which is negative, so im actually subtracting the count removed
			if (Slot.GetItemStack()->GetCount() <= 0)
			{
				TObjectPtr<UXIUItemStack> RemovedStack;
				if (Slot.Clear(RemovedStack))
				{
					if (RemovedStack) RemovedStack->SetOwningInventoryComponent(nullptr);
				}
			}
		}
	}
	return Count;
}

int FXIUInventoryList::ConsumeItem(UXIUItemDefinition* ItemDefinition, int32 Count)
{
	checkf(Count > 0, TEXT("Cannot consume negative amounts"));
	check(ItemDefinition != nullptr);
	check(CanManipulateInventory());
	
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (Slot.GetItemStack() && Slot.GetItemStack()->GetItemDefinition() == ItemDefinition)
		{
			Count += Slot.GetItemStack()->ModifyCount(-Count); // add to count the delta count which is negative, so im actually subtracting the count removed
			if (Slot.GetItemStack()->GetCount() <= 0)
			{
				TObjectPtr<UXIUItemStack> RemovedStack;
				if (Slot.Clear(RemovedStack))
				{
					if (RemovedStack) RemovedStack->SetOwningInventoryComponent(nullptr);
				}
			}
		}
	}
	return Count;
}

bool FXIUInventoryList::SetItemStackInSlot(UXIUItemStack* ItemStack, int SlotIndex, TObjectPtr<UXIUItemStack>& OldStack, bool bUpdateOwningInventory)
{
	check(CanManipulateInventory());
	checkf(SlotIndex <= Entries.Num(), TEXT("Cannot modify non existent slots"));
	
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (Slot.GetIndex() == SlotIndex)
		{
			if (Slot.SetItemStack(ItemStack, OldStack))
			{
				if (bUpdateOwningInventory && ItemStack) ItemStack->SetOwningInventoryComponent(OwnerComponent);
				return true;
			}
			break;
		}
	}
	return false;
}

bool FXIUInventoryList::ExtractItemStackFromSlot(int SlotIndex, TObjectPtr<UXIUItemStack>& ExtractedStack)
{
	check(CanManipulateInventory());
	checkf(SlotIndex <= Entries.Num(), TEXT("Cannot modify non existent slots"));
	
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (Slot.GetIndex() == SlotIndex)
		{
			return Slot.Clear(ExtractedStack);
		}
	}
	return false;
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
    	for (TObjectPtr<UXIUItemDefinition> ItemDefinition : DefaultItems)
    	{
    		AddItem(ItemDefinition, 2);
    	}
    }
}

void UXIUInventoryComponent::PrintItems()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Orange, FString::Printf(TEXT("Inventory Size: %i"), Inventory.GetSize()));
	for (const FXIUInventorySlot& Slot : Inventory.GetInventory())
	{
		if (UXIUItemStack* Stack = Slot.GetItemStack())
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Orange, FString::Printf(TEXT("[SLOT %i] Item: %s  ;  Count %i ; Fragments: %i"), Slot.GetIndex(), *Stack->GetItemDefinition()->GetItemName(), Stack->GetCount(), Stack->GetAllFragments().Num()));
		}
	}
}

UXIUItemStack* UXIUInventoryComponent::GetStackAtSlot(int32 SlotIndex)
{
	return Inventory.GetStackAtSlot(SlotIndex);
}

TArray<UXIUItemStack*> UXIUInventoryComponent::AddItem(UXIUItemDefinition* ItemDefinition, const int32 Count)
{
	TArray<UXIUItemStack*> Result;
	if (ItemDefinition != nullptr)
	{
		Inventory.AddItem(ItemDefinition, Count, Result);
	}
	return Result;
}

void UXIUInventoryComponent::AddItemStack(UXIUItemStack* ItemStack)
{
	if (ItemStack != nullptr)
	{
		Inventory.AddItemStack(ItemStack, true);
	}
}
bool UXIUInventoryComponent::ConsumeItem(UXIUItemDefinition* ItemDefinition, int32 Count)
{
	if (ItemDefinition != nullptr)
	{
		if (Inventory.ConsumeItem(ItemDefinition, Count) == 0)
		{
			return true;
		}
	}
	return false;
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