// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryComponent.h"

#include "Inventory/XIUInventoryFunctionLibrary.h"
#include "Inventory/XIUItemActor.h"
#include "Inventory/XIUItemDefinition.h"
#include "Net/UnrealNetwork.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * FXIUInventorySlot
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
 * FXIUInventory
 */


/*--------------------------------------------------------------------------------------------------------------------*/
/* FFastArraySerializer contract */

void FXIUInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FXIUInventorySlot& Slot = Entries[Index];
		
		int OldCount = Slot.GetItem() ? Slot.GetItem()->GetCount() : 0;
		RegisterSlotChange(Slot, OldCount, 0, true, Slot.GetItem());
		
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
		RegisterSlotChange(Slot, 0, NewCount, true);
		
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
		
		bool bItemChanged = Slot.LastObservedItem != Slot.GetItemSafe();
		int NewCount = Slot.GetItem() ? Slot.GetItem()->GetCount() : 0;
		int OldCount = !bItemChanged ? Slot.LastObservedCount : 0;
		RegisterSlotChange(Slot, OldCount, NewCount, bItemChanged, Slot.LastObservedItem.Get());
		
		Slot.LastObservedCount = NewCount;
		Slot.LastObservedItem = Slot.GetItemSafe();
	}
}

/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Helpers */

void FXIUInventoryList::BroadcastChangeMessage(const FXIUInventorySlot& Entry, const int32 OldCount, const int32 NewCount, const TObjectPtr<UXIUItem> OldItem) const
{
	FXIUInventorySlotChangeMessage Message;
	Message.InventoryOwner = OwnerComponent;
	Message.Index = Entry.Index;
	Message.bItemChanged = Entry.Item != OldItem || NewCount == 0;
	Message.Item = Entry.GetItemSafe();
	Message.NewCount = NewCount;
	Message.Delta = NewCount - OldCount;
	Message.OldItem = OldItem;
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
		RegisterSlotChange(NewSlot, 0, 0, true);
	}
}

int FXIUInventoryList::AddItemDefault(FXIUItemDefault ItemDefault, TArray<TObjectPtr<UXIUItem>>& AddedItems)
{
	check(CanManipulateInventory());
	checkf(ItemDefault.ItemDefinition && ItemDefault.ItemDefinition->ItemClass, TEXT("Cannot add item of not specified class"))
	
	int RemainingCount = ItemDefault.Count;
	if (RemainingCount <= 0) return RemainingCount;

	// try to add count to existing items
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (!Slot.IsEmpty() && Slot.GetItem()->GetItemDefinition() == ItemDefault.ItemDefinition)
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
		if (Slot.IsEmpty() && !Slot.IsLocked() && Slot.MatchesFilterByClass(ItemDefault.ItemDefinition->ItemClass))
		{
			ItemDefault.Count = RemainingCount;
			if (TObjectPtr<UXIUItem> NewItem = UXIUInventoryFunctionLibrary::MakeItemFromDefault(OwnerComponent->GetOwner(), ItemDefault))
			{
				TObjectPtr<UXIUItem> OldItem;
				if (Slot.SetItem(NewItem, OldItem))
				{
					MarkItemDirty(Slot);
					RegisterSlotChange(Slot, 0, NewItem->GetCount(), true, OldItem);
					
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
					RegisterSlotChange(Slot, 0, NewItem->GetCount(), true, OldItem);
					
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
			RegisterSlotChange(Slot, 0, NewItem->GetCount(), true, OldItem);
			
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
	RegisterSlotChange(Slot, OldItem ? OldItem->GetCount() : 0, 0, true, OldItem);
	
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

int FXIUInventoryList::ConsumeItemByDefinition(const TObjectPtr<UXIUItemDefinition> ItemDefinition, const int Count)
{
	check(CanManipulateInventory());
	if (!ItemDefinition) return 0;
	
	int CountLeftToConsume = Count;
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (!Slot.IsEmpty() && Slot.GetItem()->GetItemDefinition() == ItemDefinition)
		{
			CountLeftToConsume -= -Slot.GetItem()->ModifyCount(-CountLeftToConsume); // we are removing count, so the function returns a negative number representing the count removed
			
			if (CountLeftToConsume <= 0) break;
		}
	}
	return Count - CountLeftToConsume; // Consumed items
}

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Items registration */

void FXIUInventoryList::RegisterSlotChange(const FXIUInventorySlot& Slot, const int32 OldCount, const int32 NewCount, const bool bRegisterItemChange, const TObjectPtr<UXIUItem> OldItem)
{
	BroadcastChangeMessage(Slot, OldCount, NewCount, OldItem); 

	if (bRegisterItemChange)
	{
		if (UXIUItem* NewItem = Slot.GetItemSafe())
		{
			OwnerComponent->BindItemCountChangedDelegate(NewItem);
			OwnerComponent->RegisterReplicatedObject(NewItem);
		}
		if (OldItem)
		{
			OwnerComponent->UnBindItemCountChangedDelegate(OldItem);
			OwnerComponent->UnregisterReplicatedObject(OldItem);
		}
	}
}

/*--------------------------------------------------------------------------------------------------------------------*/







////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXIUInventoryComponent
 */


UXIUInventoryComponent::UXIUInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Inventory(this)
{
	PrimaryComponentTick.bCanEverTick = false;
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
		ApplySettingsToSlots();
		InputAddDefaultItems();

		bInventoryInitialized = true;
		InventoryInitializedDelegate.Broadcast();
	}
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
	if (bInventoryInitialized) InventoryInitializedDelegate.Broadcast();
}

void UXIUInventoryComponent::OnItemCountChanged(const FXIUItemCountChangeMessage& Change)
{
	checkf(Change.Item, TEXT("Item is null, which means something went really wrong"))
	
	FXIUInventorySlot ItemSlot;
	for (const FXIUInventorySlot& Slot : Inventory.GetInventory())
	{
		if (Slot.GetItem() == Change.Item)
		{
			ItemSlot = Slot;
			break;
		}
	}
	
	if (ItemSlot.GetItem())
	{
		bool bItemChanged = Change.Item->GetCount() == 0;
		Inventory.RegisterSlotChange(ItemSlot, Change.OldCount, Change.Item->GetCount(), false, bItemChanged? Change.Item : nullptr);
	}
}

void UXIUInventoryComponent::BindItemCountChangedDelegate(const TObjectPtr<UXIUItem> InItem)
{
	InItem->ItemCountChangedDelegate.AddUniqueDynamic(this, &ThisClass::OnItemCountChanged);
}

void UXIUInventoryComponent::UnBindItemCountChangedDelegate(const TObjectPtr<UXIUItem> InItem)
{
	InItem->ItemCountChangedDelegate.RemoveDynamic(this, &ThisClass::OnItemCountChanged);
}

/*--------------------------------------------------------------------------------------------------------------------*/


void UXIUInventoryComponent::ApplySettingsToSlots()
{
	
}

void UXIUInventoryComponent::InputAddDefaultItems()
{
	if (!GetOwner()) return;
	
	if (GetOwner()->HasAuthority())
	{
		AddDefaultItems();
	}
	else
	{
		ServerAddDefaultItemsRPC();
	}
}

void UXIUInventoryComponent::ServerAddDefaultItemsRPC_Implementation()
{
	AddDefaultItems();
}

void UXIUInventoryComponent::AddDefaultItems()
{
	for (const FXIUItemDefault DefaultItem : DefaultItems)
	{
		AddItemDefault(DefaultItem);
	}
}

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

void UXIUInventoryComponent::AddItemDefault(const FXIUItemDefault ItemDefault)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TArray<TObjectPtr<UXIUItem>> AddedItems;
		Inventory.AddItemDefault(ItemDefault, AddedItems);
	}
}

void UXIUInventoryComponent::AddItem(UXIUItem* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TObjectPtr<UXIUItem> AddedItem;
		Inventory.AddItem(Item, true, AddedItem);
	}
}

bool UXIUInventoryComponent::SetItemAtSlot(const int SlotIndex, UXIUItem* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TObjectPtr<UXIUItem> AddedItem;
		TObjectPtr<UXIUItem> OldItem;
		return Inventory.SetItemAtSlot(SlotIndex, Item, true, AddedItem, OldItem);
	}
	return false;
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

AXIUItemActor* UXIUInventoryComponent::DropItemAtSlot(const FTransform& DropTransform, const int SlotIndex, const int Count, const TSubclassOf<AXIUItemActor> ItemActorClass)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || Count == 0) return nullptr;

	if (UXIUItem* ItemToDrop = GetItemAtSlot(SlotIndex))
	{
		if (UWorld* World = GetWorld())
		{
			const TSubclassOf<AXIUItemActor> DropActorClass = ItemActorClass ? ItemActorClass : DefaultItemActorClass;
			AXIUItemActor* DroppedItemActor = World->SpawnActorDeferred<AXIUItemActor>(DropActorClass , DropTransform);
			DroppedItemActor->SetItem(ItemToDrop);

			if (Count > 0)
			{
				// Modify count of the item in DroppedItemActor
				const int CountToDrop = FMath::Min(ItemToDrop->GetCount(), Count);
				DroppedItemActor->Execute_GetItem(DroppedItemActor)->SetCount(CountToDrop);
				// Adjust the count in original item
				ItemToDrop->ModifyCount(-CountToDrop);
			}
			else ItemToDrop->SetCount(0); // Count = -1 so we dropped everything
			
			return DroppedItemActor;
		}
	}
	return nullptr;
}

int UXIUInventoryComponent::ConsumeItemsByDefinition(UXIUItemDefinition* ItemDefinition, const int Count)
{
	return Inventory.ConsumeItemByDefinition(ItemDefinition, Count);
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

int UXIUInventoryComponent::CountItemsByDefinition(UXIUItemDefinition* ItemDefinition)
{
	int Count = 0;
	for (const FXIUInventorySlot& Slot : Inventory.GetInventory())
	{
		if (UXIUItem* Item = Slot.GetItem())
		{
			if (Item->GetItemDefinition() == ItemDefinition) Count += Item->GetCount();
		}
	}
	return Count;
}

bool UXIUInventoryComponent::CanInsertItem(UXIUItem* Item) const
{
	for (const FXIUInventorySlot& Slot : Inventory.GetInventory())
	{
		if (Slot.CanInsertItem(Item)) return true;
	}
	return false;
}

UXIUItem* UXIUInventoryComponent::GetItemAtSlot(const int SlotIndex)
{
	return Inventory.GetItemAtSlot(SlotIndex);
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