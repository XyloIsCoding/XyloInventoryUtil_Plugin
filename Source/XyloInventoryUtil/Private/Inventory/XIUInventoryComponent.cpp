// Copyright XyloIsCoding 2024


#include "Inventory/XIUInventoryComponent.h"

#include "Inventory/XIUInventoryUtilLibrary.h"
#include "Inventory/Item/XIUItemActor.h"
#include "Inventory/Item/XIUItemDefinition.h"
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

bool FXIUInventorySlot::Clear(UXIUItem*& OldItem)
{
	if (Item)
	{
		OldItem = Item;
		Item = nullptr;
		return true;
	}
	return false;
}

bool FXIUInventorySlot::SetItem(UXIUItem* NewItem, UXIUItem*& OldItem)
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

UXIUItem* FXIUInventorySlot::GetItem() const
{
	return Item;
}

UXIUItem* FXIUInventorySlot::GetItemSafe() const
{
	return IsEmpty() ? nullptr : Item;
}

bool FXIUInventorySlot::IsEmpty() const
{
	return !UXIUItem::IsItemAvailable(Item); 
}

int32 FXIUInventorySlot::GetItemCountSafe() const
{
	return UXIUItem::IsItemInitialized(GetItem()) ? GetItem()->GetCount() : 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Filter */

void FXIUInventorySlot::SetFilter(const TSubclassOf<UXIUItem> NewFilter)
{
	Filter = NewFilter;
}

bool FXIUInventorySlot::MatchesFilter(const UXIUItem* TestItem) const
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

bool FXIUInventorySlot::CanInsertItem(UXIUItem* TestItem) const
{
	if (IsLocked()) return false;
	if (IsEmpty()) return MatchesFilter(TestItem);
	if (!Item->IsFull() && Item->CanStack(TestItem)) return true;
	return false;
}

void FXIUInventorySlot::ApplySettings(const FXIUInventorySlotSettings& SlotSettings)
{
	Filter = SlotSettings.Filter;
	bLocked = SlotSettings.bLocked;
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

void FXIUInventoryList::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FXIUInventorySlot& Slot = Entries[Index];
		
		int OldCount = Slot.GetItemCountSafe();
		RegisterSlotChange(Slot, OldCount, 0, true, Slot.GetItem());
		
		Slot.LastObservedCount = 0;
		Slot.LastObservedItem = nullptr;
	}
}

void FXIUInventoryList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FXIUInventorySlot& Slot = Entries[Index];
		
		int NewCount = Slot.GetItemCountSafe();
		RegisterSlotChange(Slot, 0, NewCount, true);
		
		Slot.LastObservedCount = NewCount;
		Slot.LastObservedItem = Slot.GetItemSafe();
	}
}

void FXIUInventoryList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FXIUInventorySlot& Slot = Entries[Index];
		check(Slot.LastObservedCount != INDEX_NONE);

		int NewCount = Slot.GetItemCountSafe();
		bool bItemChanged = Slot.LastObservedItem != Slot.GetItem() || (Slot.LastObservedCount != 0 && NewCount == 0);
		int OldCount = !bItemChanged ? Slot.LastObservedCount : 0;
		RegisterSlotChange(Slot, OldCount, NewCount, bItemChanged, Slot.LastObservedItem.Get());
		
		Slot.LastObservedCount = NewCount;
		Slot.LastObservedItem = Slot.GetItemSafe();
	}
}

/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Helpers */

void FXIUInventoryList::BroadcastChangeMessage(const FXIUInventorySlot& Entry, const int32 OldCount, const int32 NewCount, UXIUItem* OldItem) const
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

	if (OwnerComponent) OwnerComponent->BroadcastInventoryChanged(Message);
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

void FXIUInventoryList::AddSlot(const FXIUInventorySlotSettings& SlotSettings)
{
	check(CanManipulateInventory());

	FXIUInventorySlot& NewSlot = Entries.AddDefaulted_GetRef();
	NewSlot.Index = Entries.Num() - 1; // we can never remove slots, so indexes are for sure progressive
	NewSlot.ApplySettings(SlotSettings);
	MarkItemDirty(NewSlot);
	RegisterSlotChange(NewSlot, 0, 0, true);
}

int FXIUInventoryList::AddItemDefault(FXIUItemDefault ItemDefault, TArray<UXIUItem*>& AddedItems)
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
			if (UXIUItem* NewItem = UXIUInventoryUtilLibrary::MakeItemFromDefault(OwnerComponent->GetOwner(), ItemDefault))
			{
				UXIUItem* OldItem;
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

int FXIUInventoryList::AddItem(UXIUItem* Item, int32 CountOverride, bool bDuplicate, bool bModifyItemCount, UXIUItem*& AddedItem)
{
	check(CanManipulateInventory());
	checkf(Item, TEXT("Cannot add item invalid item"))

	int RemainingCount = CountOverride >= 0 ? FMath::Min(Item->GetCount(), CountOverride) : Item->GetCount();
	if (RemainingCount <= 0) return 0;

	// try to add count to existing items
	for (FXIUInventorySlot& Slot : Entries)
	{
		if (!Slot.IsEmpty() && Slot.GetItem()->CanStack(Item))
		{
			RemainingCount -= Slot.GetItem()->ModifyCount(RemainingCount);

			// We are done adding, and we do not need to create a new stack
			if (RemainingCount <= 0)
			{
				if (bModifyItemCount) Item->SetCount(0);
				return RemainingCount;
			}
		}
	}

	// still count to add, so we make new item
	if (bModifyItemCount) Item->SetCount(RemainingCount);
	if (UXIUItem* NewItem = bDuplicate ? UXIUInventoryUtilLibrary::DuplicateItem(OwnerComponent->GetOwner(), Item) : Item)
	{
		NewItem->SetCount(RemainingCount);
		for (FXIUInventorySlot& Slot : Entries)
		{
			if (Slot.IsEmpty())
			{
				UXIUItem* OldItem;
				if (Slot.SetItem(NewItem, OldItem))
				{
					MarkItemDirty(Slot);
					RegisterSlotChange(Slot, 0, NewItem->GetCount(), true, OldItem);
					
					AddedItem = NewItem;
					if (bDuplicate && bModifyItemCount) Item->SetCount(0);
					RemainingCount = 0;
					break;
				}
			}
		}
	}
	return RemainingCount;
}

bool FXIUInventoryList::SetItemAtSlot(int SlotIndex, UXIUItem* Item, bool bDuplicate, UXIUItem*& AddedItem, UXIUItem*& OldItem)
{
	check(CanManipulateInventory());
	checkf(SlotIndex < Entries.Num(), TEXT("The slot at index %i does not exist"), SlotIndex)
	
	if (UXIUItem* NewItem = bDuplicate ? UXIUInventoryUtilLibrary::DuplicateItem(OwnerComponent->GetOwner(), Item) : Item)
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

UXIUItem* FXIUInventoryList::GetItemAtSlot(const int SlotIndex)
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

UXIUItem* FXIUInventoryList::RemoveItemAtSlot(int SlotIndex)
{
	check(CanManipulateInventory());
	checkf(SlotIndex < Entries.Num(), TEXT("The slot at index %i does not exist"), SlotIndex)

	UXIUItem* OldItem;
	FXIUInventorySlot& Slot = Entries[SlotIndex];
	Slot.Clear(OldItem);

	MarkItemDirty(Slot);
	RegisterSlotChange(Slot, OldItem ? OldItem->GetCount() : 0, 0, true, OldItem);
	
	return OldItem;
}

bool FXIUInventoryList::GetItemsByClass(const TSubclassOf<UXIUItem> ItemClass, TArray<UXIUItem*>& FoundItems)
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

int FXIUInventoryList::ConsumeItemByDefinition(const UXIUItemDefinition* ItemDefinition, const int Count)
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

void FXIUInventoryList::RegisterSlotChange(const FXIUInventorySlot& Slot, const int32 OldCount, const int32 NewCount, const bool bRegisterItemChange, UXIUItem* OldItem)
{
	if (bRegisterItemChange)
	{
		// if new item is not initialized, we register it, and we bind the ItemInitialized to receive a change when it
		// gets initialized.
		// otherwise if the item is not empty, we register it, and we bind the count changed delegate
		if (UXIUItem* NewItem = Slot.GetItem())
		{
			if (!NewItem->IsItemInitialized())
			{
				OwnerComponent->RegisterReplicatedObject(NewItem);
				OwnerComponent->BindItemInitializedDelegate(NewItem);
			}
			else
			{
				OwnerComponent->UnBindItemInitializedDelegate(NewItem);
				if (!NewItem->IsEmpty())
				{
					OwnerComponent->RegisterReplicatedObject(NewItem);
					OwnerComponent->BindItemCountChangedDelegate(NewItem);
				}
			}
		}
	}

	// Broadcast change
	BroadcastChangeMessage(Slot, OldCount, NewCount, OldItem); 

	if (bRegisterItemChange)
	{
		// if the old item is valid, we unregister it and unbind the ItemCountChanged delegate
		if (OldItem)
		{
			OwnerComponent->UnBindItemCountChangedDelegate(OldItem);
			OwnerComponent->UnregisterReplicatedObject(OldItem, true);
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
	InventorySize = 1;
	bManualInitialization = false;
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
		if (bManualInitialization)
		{
			ManualInitialization();
		}
		else
		{
			Inventory.InitInventory(InventorySize);
		}
		AddDefaultItems();
		SetInventoryInitialized(true);
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

void UXIUInventoryComponent::SetInventoryInitialized(bool bInitialized)
{
	bInventoryInitialized = bInitialized;
	OnRep_InventoryInitialized();
}

void UXIUInventoryComponent::OnRep_InventoryInitialized()
{
	if (bInventoryInitialized)
	{
		BP_OnInventoryInitialized();
		InventoryInitializedDelegate.Broadcast();
	}
}

void UXIUInventoryComponent::BroadcastInventoryChanged(const FXIUInventorySlotChangeMessage& Message)
{
	BP_OnInventoryChanged();
	InventoryChangedDelegate.Broadcast(Message);
}

void UXIUInventoryComponent::BindItemCountChangedDelegate(UXIUItem* InItem)
{
	InItem->ItemCountChangedDelegate.AddUniqueDynamic(this, &ThisClass::OnItemCountChanged);
}

void UXIUInventoryComponent::UnBindItemCountChangedDelegate(UXIUItem* InItem)
{
	InItem->ItemCountChangedDelegate.RemoveDynamic(this, &ThisClass::OnItemCountChanged);
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
		if (GetOwner() && GetOwner()->HasAuthority())
		{
			Inventory.RegisterSlotChange(ItemSlot, Change.OldCount, Change.Item->GetCount(), bItemChanged, bItemChanged? Change.Item : nullptr);
		}
		else
		{
			TArray<int32> ChangedIndex = { ItemSlot.GetIndex() };
			Inventory.PostReplicatedChange(ChangedIndex, Inventory.GetSize());
		}
	}
}

void UXIUInventoryComponent::OnItemInitialized(UXIUItem* InItem)
{
	checkf(InItem, TEXT("Item is null, which means something went really wrong"))
	
	FXIUInventorySlot ItemSlot;
	for (const FXIUInventorySlot& Slot : Inventory.GetInventory())
	{
		if (Slot.GetItem() == InItem)
		{
			ItemSlot = Slot;
			break;
		}
	}
	
	if (ItemSlot.GetItem())
	{
		if (GetOwner() && GetOwner()->HasAuthority())
		{
			Inventory.RegisterSlotChange(ItemSlot, 0, InItem->GetCount(), true, nullptr);
		}
		else
		{
			TArray<int32> ChangedIndex = { ItemSlot.GetIndex() };
			Inventory.PostReplicatedChange(ChangedIndex, Inventory.GetSize());
		}
	}
}

void UXIUInventoryComponent::BindItemInitializedDelegate(UXIUItem* InItem)
{
	InItem->ItemInitializedDelegate.AddUniqueDynamic(this, &ThisClass::OnItemInitialized);
}

void UXIUInventoryComponent::UnBindItemInitializedDelegate(UXIUItem* InItem)
{
	InItem->ItemInitializedDelegate.RemoveDynamic(this, &ThisClass::OnItemInitialized);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void UXIUInventoryComponent::ManualInitialization()
{
	BP_OnManualInitialization();
	ManualInitializationDelegate.Broadcast();
	// to be implemented in child classes
}

void UXIUInventoryComponent::AddSlot(const FXIUInventorySlotSettings& SlotSettings)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Inventory.AddSlot(SlotSettings);
	}
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
			if (UXIUItem* Item = Slot.GetItem())
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
		TArray<UXIUItem*> AddedItems;
		Inventory.AddItemDefault(ItemDefault, AddedItems);
	}
}

void UXIUInventoryComponent::AddItem(UXIUItem* Item, int32 CountOverride)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		UXIUItem* AddedItem;
		Inventory.AddItem(Item, CountOverride, true, true, AddedItem);
	}
}

void UXIUInventoryComponent::AddItemNoModify(UXIUItem* Item, int32 CountOverride)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		UXIUItem* AddedItem;
		Inventory.AddItem(Item, CountOverride, true, false, AddedItem);
	}
}

bool UXIUInventoryComponent::SetItemAtSlot(const int32 SlotIndex, UXIUItem* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		UXIUItem* AddedItem;
		UXIUItem* OldItem;
		return Inventory.SetItemAtSlot(SlotIndex, Item, true, AddedItem, OldItem);
	}
	return false;
}

void UXIUInventoryComponent::TransferItemFromSlot(int SlotIndex, UXIUInventoryComponent* OtherInventory)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (UXIUItem* Item = Inventory.GetItemAtSlot(SlotIndex))
		{
			// we can just use add item since Inventory.AddItem already takes care of modifying Item count
			OtherInventory->AddItem(Item);
		}
	}
}

AXIUItemActor* UXIUInventoryComponent::DropItemAtSlot(const FTransform& DropTransform, const int32 SlotIndex, const int32 Count, const TSubclassOf<AXIUItemActor> ItemActorClass, const bool bFinishSpawning)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || Count == 0) return nullptr;

	if (UXIUItem* ItemToDrop = GetItemAtSlot(SlotIndex))
	{
		if (UWorld* World = GetWorld())
		{
			const TSubclassOf<AXIUItemActor> DropActorClass = ItemActorClass ? ItemActorClass : DefaultItemActorClass;
			AXIUItemActor* DroppedItemActor = World->SpawnActorDeferred<AXIUItemActor>(DropActorClass , DropTransform);
			if (!DroppedItemActor) return nullptr;

			// if Count < 0 drop everything, else try to drop Count if we have enough
			const int CountToDrop = Count > 0 ? FMath::Min(ItemToDrop->GetCount(), Count) : ItemToDrop->GetCount();
			// Set Item in dropped item actor
			DroppedItemActor->Execute_SetItem(DroppedItemActor, ItemToDrop, CountToDrop);
			// Adjust the count in original item
			ItemToDrop->ModifyCount(-CountToDrop);

			if (bFinishSpawning)
			{
				DroppedItemActor->FinishSpawning(DropTransform);
			}
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
		if (UXIUItem* Item = Slot.GetItemSafe())
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

