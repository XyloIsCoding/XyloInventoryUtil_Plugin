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

TObjectPtr<UXIUItem> FXIUInventorySlot::GetItemSafe() const
{
	return IsEmpty() ? nullptr : Item;
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

bool FXIUInventorySlot::MatchesFilterByClass(const TSubclassOf<UXIUItem> TestItemClass) const
{
	return !Filter || TestItemClass && (TestItemClass->IsChildOf(Filter));
}

/*--------------------------------------------------------------------------------------------------------------------*/
	
/*--------------------------------------------------------------------------------------------------------------------*/
/* Locked */

void FXIUInventorySlot::SetLocked(bool NewLock)
{
	bLocked = NewLock;
}

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Generic */

bool FXIUInventorySlot::CanInsertItem(const TObjectPtr<UXIUItem> TestItem) const
{
	if (IsLocked()) return false;
	if (IsEmpty() && MatchesFilter(TestItem)) return true;
	if (!Item->IsFull() && Item->CanStack(TestItem)) return true;
	return false;
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
		
		int OldCount = Slot.GetItem() ? Slot.GetItem()->GetCount() : 0;
		BroadcastChangeMessage(Slot, /*OldCount=*/ OldCount, /*NewCount=*/ 0, true);

		if (Slot.Item) OwnerComponent->UnBindItemCountChangedDelegate(Slot.Item);
		Slot.LastObservedCount = 0;
		Slot.LastObservedItem = nullptr;
	}
}

void FXIUInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FXIUInventorySlot& Slot = Entries[Index];
		
		int NewCount = Slot.GetItem() ? Slot.GetItem()->GetCount() : 0;
		BroadcastChangeMessage(Slot, /*OldCount=*/ 0, /*NewCount=*/ NewCount, true);

		if (Slot.Item) OwnerComponent->BindItemCountChangedDelegate(Slot.Item);
		Slot.LastObservedCount = NewCount;
		Slot.LastObservedItem = Slot.GetItemSafe();
	}
}

void FXIUInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FXIUInventorySlot& Slot = Entries[Index];
		check(Slot.LastObservedCount != INDEX_NONE);

		int NewCount = Slot.GetItem() ? Slot.GetItem()->GetCount() : 0;
		bool bItemChanged = Slot.LastObservedItem != Slot.GetItemSafe();
		int OldCount = !bItemChanged ? Slot.LastObservedCount : 0;
		BroadcastChangeMessage(Slot, /*OldCount=*/ OldCount, /*NewCount=*/ NewCount, bItemChanged);

		if (bItemChanged)
		{
			if (Slot.LastObservedItem) OwnerComponent->UnBindItemCountChangedDelegate(Slot.LastObservedItem.Get());
			if (Slot.Item) OwnerComponent->BindItemCountChangedDelegate(Slot.Item);
		}
		Slot.LastObservedCount = NewCount;
		Slot.LastObservedItem = Slot.GetItemSafe();
	}
}

/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Helpers */

void FXIUInventoryList::BroadcastChangeMessage(const FXIUInventorySlot& Entry, const int32 OldCount, const int32 NewCount, const bool bItemChanged) const
{
	FXIUInventorySlotChangeMessage Message;
	Message.InventoryOwner = OwnerComponent;
	Message.Index = Entry.Index;
	Message.bItemChanged = bItemChanged;
	Message.Item = Entry.GetItemSafe();
	Message.NewCount = NewCount;
	Message.Delta = NewCount - OldCount;
	Message.Filter = Entry.Filter;
	Message.bLocked = Entry.bLocked;

	if (OwnerComponent) OwnerComponent->InventoryChangedDelegate.Broadcast(Message);
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
		BroadcastChangeMessage(NewSlot, 0, 0, true); // Broadcast on server
		if (NewSlot.Item) OwnerComponent->BindItemCountChangedDelegate(NewSlot.Item); // Bind on server
	}
}

int FXIUInventoryList::AddItemDefault(FXIUItemDefault ItemDefault, TArray<TObjectPtr<UXIUItem>>& AddedItems)
{
	check(CanManipulateInventory());
	checkf(ItemDefault.ItemClass, TEXT("Cannot add item of not specified class"))
	
	int RemainingCount = ItemDefault.Count;
	if (RemainingCount <= 0) return RemainingCount;

	// try to add count to existing items
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (!Slot.IsEmpty() && Slot.GetItem().IsA(ItemDefault.ItemClass))
		{
			RemainingCount -= Slot.GetItem()->ModifyCount(RemainingCount);
			
			if (RemainingCount <= 0)
			{
				return RemainingCount;
			}
		}
	}
	
	
	// still count to add
	for (FXIUInventorySlot& Slot : Entries)
	{
		//check if I can add the item
		if (Slot.IsEmpty() && !Slot.IsLocked() && Slot.MatchesFilterByClass(ItemDefault.ItemClass))
		{
			ItemDefault.Count = RemainingCount;
			if (TObjectPtr<UXIUItem> NewItem = UXIUInventoryFunctionLibrary::MakeItemFromDefault(OwnerComponent->GetOwner(), ItemDefault))
			{
				TObjectPtr<UXIUItem> OldItem;
				if (Slot.SetItem(NewItem, OldItem))
				{
					MarkItemDirty(Slot);
					BroadcastChangeMessage(Slot, 0, NewItem->GetCount(), true); // Broadcast on server
					OwnerComponent->BindItemCountChangedDelegate(NewItem); // Bind on server
					
					AddedItems.Add(NewItem);
					RemainingCount -= NewItem->GetCount();
					
					if (RemainingCount <= 0)
					{
						return RemainingCount;
					}
				}
			}
		}
	}
	return RemainingCount;
}

int FXIUInventoryList::AddItem(TObjectPtr<UXIUItem> Item, bool bDuplicate, TObjectPtr<UXIUItem>& AddedItem)
{
	check(CanManipulateInventory());
	checkf(Item, TEXT("Cannot add item invalid item"))

	int RemainingCount = Item->GetCount();
	if (RemainingCount <= 0) return RemainingCount;

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
	if (TObjectPtr<UXIUItem> NewItem = bDuplicate ? UXIUInventoryFunctionLibrary::DuplicateItem(OwnerComponent->GetOwner(), Item) : Item.Get())
	{
		for (FXIUInventorySlot& Slot : Entries)
		{
			if (Slot.IsEmpty())
			{
				TObjectPtr<UXIUItem> OldItem;
				if (Slot.SetItem(NewItem, OldItem))
				{
					MarkItemDirty(Slot);
					BroadcastChangeMessage(Slot, 0, NewItem->GetCount(), true); // Broadcast on server
					OwnerComponent->BindItemCountChangedDelegate(NewItem); // Bind on server
					
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
	
	if (TObjectPtr<UXIUItem> NewItem = bDuplicate ? UXIUInventoryFunctionLibrary::DuplicateItem(OwnerComponent->GetOwner(), Item) : Item.Get())
	{
		FXIUInventorySlot& Slot = Entries[SlotIndex];
		if (Slot.SetItem(NewItem, OldItem))
		{
			MarkItemDirty(Slot);
			BroadcastChangeMessage(Slot, 0, NewItem->GetCount(), true); // Broadcast on server
			OwnerComponent->BindItemCountChangedDelegate(NewItem); // Bind on server
			
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
			return Slot.GetItemSafe();
		}
	}
	return nullptr;
}

TObjectPtr<UXIUItem> FXIUInventoryList::RemoveItemAtSlot(int SlotIndex)
{
	check(CanManipulateInventory());
	checkf(SlotIndex < Entries.Num(), TEXT("The slot at index %i does not exist"), SlotIndex)

	TObjectPtr<UXIUItem> OldItem;
	FXIUInventorySlot& Slot = Entries[SlotIndex];
	Slot.Clear(OldItem);

	MarkItemDirty(Slot);
	int OldCount = OldItem ? OldItem->GetCount() : 0;
	BroadcastChangeMessage(Slot, OldCount, 0, true); // Broadcast on server
	OwnerComponent->UnBindItemCountChangedDelegate(OldItem); // UnBind on server
	
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
	int RemainingCount = Count;

	// try to add count to existing items
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (!Slot.IsEmpty() && Slot.GetItem().IsA(ItemClass))
		{
			RemainingCount -= -Slot.GetItem()->ModifyCount(-RemainingCount); // we are removing count, so the function returns a negative number representing the count removed
			
			if (RemainingCount <= 0) break;
		}
	}
	return Count - RemainingCount; // Consumed items
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

		bInventoryInitialized = true;
		InventoryInitializedDelegate.Broadcast();
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
	DOREPLIFETIME(ThisClass, bInventoryInitialized);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Inventory
 */



/*--------------------------------------------------------------------------------------------------------------------*/
/* Delegates */

void UXIUInventoryComponent::OnRep_InventoryInitialized()
{
	InventoryInitializedDelegate.Broadcast();
}

void UXIUInventoryComponent::OnItemCountChanged(const FXIUItemCountChangeMessage& Change)
{
	checkf(Change.Item, TEXT("Item is null, Which means something went really wrong"))
	
	if (Change.NewCount == 0)
	{
		UnBindItemCountChangedDelegate(Change.Item);
	}

	for (const FXIUInventorySlot& Slot : Inventory.GetInventory())
	{
		if (Slot.GetItem() == Change.Item)
		{
			Inventory.BroadcastChangeMessage(Slot, Change.OldCount, Change.NewCount, Change.NewCount != 0);
			break;
		}
	}
}

void UXIUInventoryComponent::BindItemCountChangedDelegate(const TObjectPtr<UXIUItem> InItem)
{
	InItem->ItemCountChangedDelegate.AddDynamic(this, &ThisClass::OnItemCountChanged);
}

void UXIUInventoryComponent::UnBindItemCountChangedDelegate(const TObjectPtr<UXIUItem> InItem)
{
	InItem->ItemCountChangedDelegate.RemoveDynamic(this, &ThisClass::OnItemCountChanged);
}

/*--------------------------------------------------------------------------------------------------------------------*/


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
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TArray<TObjectPtr<UXIUItem>> AddedItems;
		Inventory.AddItemDefault(ItemDefault, AddedItems);
	
		for (TObjectPtr<UXIUItem> ItemToRegister : AddedItems)
		{
			RegisterReplicatedObject(ItemToRegister);
		}
	}
}

void UXIUInventoryComponent::AddItem(UXIUItem* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TObjectPtr<UXIUItem> AddedItem;
		Inventory.AddItem(Item, true, AddedItem);

		if (AddedItem) RegisterReplicatedObject(AddedItem);
	}
}

void UXIUInventoryComponent::TransferItemFromSlot(int SlotIndex, UXIUInventoryComponent* OtherInventory)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (TObjectPtr<UXIUItem> Item = Inventory.GetItemAtSlot(SlotIndex))
		{
			// we can just use add item since Inventory.AddItem already takes care of modifying Item count
			OtherInventory->AddItem(Item);
		}
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

bool UXIUInventoryComponent::CanInsertItem(UXIUItem* Item) const
{
	for (const FXIUInventorySlot& Slot : Inventory.GetInventory())
	{
		if (Slot.CanInsertItem(Item)) return true;
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