// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryComponent.h"

#include "Inventory/XIUInventoryFunctionLibrary.h"
#include "Net/UnrealNetwork.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * FXIUInventorySlot Interface
 */


FString FXIUInventorySlot::GetDebugString() const
{
	if (Item)
	{
		return FString::Printf(TEXT("%s (%i x %s)"), *GetNameSafe(Item), Item->GetCount(), *Item->GetItemName());
	}
	return FString::Printf(TEXT("Empty"));
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Stack */

bool FXIUInventorySlot::Clear(TObjectPtr<UXIUItem>& OldItem)
{
	if (Item)
	{
		OldItem = Item;
		Item = nullptr;
		return true;
	}
	return false;
}

bool FXIUInventorySlot::SetItem(const TObjectPtr<UXIUItem> NewItem, TObjectPtr<UXIUItem>& OldItem)
{
	if (bLocked) return false;
	
	if (MatchesFilter(NewItem))
	{
		OldItem = Item;
		Item = NewItem;
		return true;
	}
	return false;
}

TObjectPtr<UXIUItem> FXIUInventorySlot::GetItem() const
{
	return Item;
}

bool FXIUInventorySlot::IsEmpty() const
{
	return Item == nullptr || Item->IsEmpty(); 
}

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Filter */

void FXIUInventorySlot::SetFilter(const TSubclassOf<UXIUItem> NewFilter)
{
	Filter = NewFilter;
}

bool FXIUInventorySlot::MatchesFilter(const TObjectPtr<UXIUItem> TestItem) const
{
	return !Filter || !TestItem || (TestItem->IsA(Filter));
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
	for (int32 Index : RemovedIndices)
	{
		FXIUInventorySlot& Slot = Entries[Index];
		BroadcastChangeMessage(Slot, /*OldCount=*/ Slot.GetItem()->GetCount(), /*NewCount=*/ 0);
		Slot.LastObservedCount = 0;
	}
}

void FXIUInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FXIUInventorySlot& Slot = Entries[Index];
		BroadcastChangeMessage(Slot, /*OldCount=*/ 0, /*NewCount=*/ Slot.GetItem()->GetCount());
		Slot.LastObservedCount = Slot.GetItem()->GetCount();
	}
}

void FXIUInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FXIUInventorySlot& Slot = Entries[Index];
		check(Slot.LastObservedCount != INDEX_NONE);
		BroadcastChangeMessage(Slot, /*OldCount=*/ Slot.LastObservedCount, /*NewCount=*/ Slot.GetItem()->GetCount());
		Slot.LastObservedCount = Slot.GetItem()->GetCount();
	}
}

/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Helpers */

void FXIUInventoryList::BroadcastChangeMessage(const FXIUInventorySlot& Entry, const int32 OldCount, const int32 NewCount) const
{
	FXIUInventoryChangeMessage Message;
	Message.InventoryOwner = OwnerComponent;
	Message.Item = Entry.GetItem();
	Message.NewCount = NewCount;
	Message.Delta = NewCount - OldCount;

	if (OwnerComponent) OwnerComponent->InventoryReplicatedDelegate.Broadcast(Message);
}

bool FXIUInventoryList::CanManipulateInventory() const
{
	if (!OwnerComponent) return false;
	AActor* OwningActor = OwnerComponent->GetOwner();
	if (!OwningActor->HasAuthority()) return false;
	return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Slots Management */

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

int FXIUInventoryList::AddItemDefault(FXIUItemDefault ItemDefault, TArray<TObjectPtr<UXIUItem>>& AddedItems)
{
	check(CanManipulateInventory());

	UE_LOG(LogTemp, Warning, TEXT("Adding count %i"), ItemDefault.Count)
	int RemainingCount = ItemDefault.Count;

	// try to add count to existing items
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (!Slot.IsEmpty() && Slot.GetItem().IsA(ItemDefault.ItemClass))
		{
			RemainingCount -= Slot.GetItem()->ModifyCount(RemainingCount);

			if (RemainingCount <= 0) return RemainingCount;
		}
	}

	// still count to add, so we make new item
	while (RemainingCount > 0)
	{
		ItemDefault.Count = RemainingCount;
		if (TObjectPtr<UXIUItem> NewItem = UXIUInventoryFunctionLibrary::MakeItemFromDefault(OwnerComponent, ItemDefault))
		{
			UE_LOG(LogTemp, Warning, TEXT("Created new item"))
			for (FXIUInventorySlot& Slot : Entries)
			{
				if (Slot.IsEmpty())
				{
					TObjectPtr<UXIUItem> OldItem;
					if (Slot.SetItem(NewItem, OldItem))
					{
						UE_LOG(LogTemp, Warning, TEXT("Added new item"))
						AddedItems.Add(NewItem);
						RemainingCount -= NewItem->GetCount();
						break;
					}
				}
			}
		}
		break; // TODO: remove
	}
	return RemainingCount;
}

int FXIUInventoryList::AddItem(TObjectPtr<UXIUItem> Item, bool bDuplicate, TObjectPtr<UXIUItem>& AddedItem)
{
	check(CanManipulateInventory());

	int RemainingCount = Item->GetCount();

	// try to add count to existing items
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (!Slot.IsEmpty() && Slot.GetItem()->CanStack(Item))
		{
			RemainingCount -= Slot.GetItem()->ModifyCount(RemainingCount);

			if (RemainingCount <= 0)
			{
				Item->SetCount(RemainingCount);
				return RemainingCount;
			}
		}
	}

	// still count to add, so we make new item
	Item->SetCount(RemainingCount);
	if (TObjectPtr<UXIUItem> NewItem = bDuplicate ? UXIUInventoryFunctionLibrary::DuplicateItem(OwnerComponent, Item) : Item.Get())
	{
		for (FXIUInventorySlot& Slot : Entries)
		{
			if (Slot.IsEmpty())
			{
				TObjectPtr<UXIUItem> OldItem;
				if (Slot.SetItem(NewItem, OldItem))
				{
					AddedItem = NewItem;
					if (bDuplicate) Item->SetCount(0);
					RemainingCount = 0;
					break;
				}
			}
		}
	}
	return RemainingCount;
}

bool FXIUInventoryList::SetItemAtSlot(int SlotIndex, TObjectPtr<UXIUItem> Item, bool bDuplicate, TObjectPtr<UXIUItem>& AddedItem, TObjectPtr<UXIUItem>& OldItem)
{
	check(CanManipulateInventory());
	checkf(SlotIndex < Entries.Num(), TEXT("The slot at index %i does not exist"), SlotIndex)

	if (TObjectPtr<UXIUItem> NewItem = bDuplicate ? UXIUInventoryFunctionLibrary::DuplicateItem(OwnerComponent, Item) : Item.Get())
	{
		if (Entries[SlotIndex].SetItem(NewItem, OldItem))
		{
			AddedItem = NewItem;
			return true;
		}
	}
	return false;
}

TObjectPtr<UXIUItem> FXIUInventoryList::GetItemAtSlot(const int SlotIndex)
{
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (Slot.GetIndex() == SlotIndex)
		{
			return !Slot.IsEmpty() ? Slot.GetItem() : nullptr;
		}
	}
	return nullptr;
}

TObjectPtr<UXIUItem> FXIUInventoryList::RemoveItemAtSlot(int SlotIndex)
{
	check(CanManipulateInventory());
	checkf(SlotIndex < Entries.Num(), TEXT("The slot at index %i does not exist"), SlotIndex)

	TObjectPtr<UXIUItem> OldItem;
	Entries[SlotIndex].Clear(OldItem);
	return OldItem;
}

bool FXIUInventoryList::GetItemsByClass(const TSubclassOf<UXIUItem> ItemClass, TArray<TObjectPtr<UXIUItem>>& FoundItems)
{
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (!Slot.IsEmpty() && Slot.GetItem()->IsA(ItemClass))
		{
			FoundItems.Add(Slot.GetItem());
		}
	}
	return FoundItems.Num() > 0;
}

int FXIUInventoryList::ConsumeItemByClass(const TSubclassOf<UXIUItem> ItemClass, const int Count)
{
	int ConsumedItems = 0;
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (!Slot.IsEmpty() && Slot.GetItem()->IsA(ItemClass))
		{
			ConsumedItems += -Slot.GetItem()->ModifyCount(-Count); // we are removing count, so the function returns a negative number representing the count removed
		}
	}
	return ConsumedItems;
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

void UXIUInventoryComponent::PrintItems()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Orange, FString::Printf(TEXT("Inventory Size: %i"), Inventory.GetSize()));
	for (const FXIUInventorySlot& Slot : Inventory.GetInventory())
	{
		if (!Slot.IsEmpty())
		{
			if (TObjectPtr<UXIUItem> Item = Slot.GetItem())
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Orange, FString::Printf(TEXT("[SLOT %i] Item: %s  ;  Count %i"), Slot.GetIndex(), *Item->GetItemName(), Item->GetCount()));
			}
		}
	}
}

void UXIUInventoryComponent::AddDefaultItems()
{
	ServerAddDefaultItems();
}

void UXIUInventoryComponent::ServerAddDefaultItems_Implementation()
{
	if (GetOwner() && GetOwner()->HasAuthority())
    {
    	for (const FXIUItemDefault DefaultItem : DefaultItems)
    	{
    		AddItemDefault(DefaultItem);
    	}
    }
}

void UXIUInventoryComponent::AddItemDefault(const FXIUItemDefault ItemDefault)
{
	TArray<TObjectPtr<UXIUItem>> AddedItems;
	Inventory.AddItemDefault(ItemDefault, AddedItems);
	
	for (TObjectPtr<UXIUItem> ItemToRegister : AddedItems)
	{
		RegisterReplicatedObject(ItemToRegister);
	}
}

UXIUItem* UXIUInventoryComponent::GetFirstItem()
{
	for (const FXIUInventorySlot& Slot : Inventory.GetInventory())
	{
		if (!Slot.IsEmpty())
		{
			return Slot.GetItem();
		}
	}
	return nullptr;
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