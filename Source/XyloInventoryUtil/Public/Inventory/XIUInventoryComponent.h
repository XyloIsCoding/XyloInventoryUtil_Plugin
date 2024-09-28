// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XIUItem.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "XIUInventoryComponent.generated.h"


struct FXIUInventoryList;
class UXIUInventoryComponent;

USTRUCT(BlueprintType)
struct FXIUInventorySlotChangeMessage
{
	GENERATED_BODY()

	//@TODO: Tag based names+owning actors for inventories instead of directly exposing the component?
	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	TObjectPtr<UActorComponent> InventoryOwner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 Index = 0;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	bool bItemChanged = false;
	
	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TObjectPtr<UXIUItem> Item = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 NewCount = 0;

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 Delta = 0;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TSubclassOf<UXIUItem> Filter = nullptr;

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	bool bLocked = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FXIUInventoryChangedSignature, const FXIUInventorySlotChangeMessage&, Change);



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * FXIUInventorySlot
 */

/** A single entry in an inventory.
 * Use IsEmpty() to check if there is a valid item or not, since items are to be ignored if count == 0 (Item.IsEmpty) */
USTRUCT(BlueprintType)
struct FXIUInventorySlot : public FFastArraySerializerItem
{
	GENERATED_BODY()

private:
	friend FXIUInventoryList;
	
public:
	FXIUInventorySlot()
		: Index(-1),
		  bLocked(false)
	{
	}

public:
	FString GetDebugString() const;

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Index */
	
private:
	UPROPERTY()
	int Index;
public:
	int GetIndex() const { return Index; }

/*--------------------------------------------------------------------------------------------------------------------*/
	
/*--------------------------------------------------------------------------------------------------------------------*/
	/* Item */
	
private:
	UPROPERTY()
	TObjectPtr<UXIUItem> Item = nullptr;
public:
	/** @return true if item got modified */
	bool Clear(TObjectPtr<UXIUItem>& OldItem);
	/** @return true if item got modified */
	bool SetItem(const TObjectPtr<UXIUItem> NewItem, TObjectPtr<UXIUItem>& OldItem);
	/** does NOT check for IsEmpty on the item
	 * @return item of this slot. */
	TObjectPtr<UXIUItem> GetItem() const;
	/** DOES check for IsEmpty on the item
	 * @return item of this slot. */
	TObjectPtr<UXIUItem> GetItemSafe() const;
	bool IsEmpty() const;

	UPROPERTY(NotReplicated)
	TSoftObjectPtr<UXIUItem> LastObservedItem = nullptr;
	UPROPERTY(NotReplicated)
	int32 LastObservedCount = INDEX_NONE;
	
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Filter */
	
private:
	UPROPERTY()
	TSubclassOf<UXIUItem> Filter;
public:
	void SetFilter(const TSubclassOf<UXIUItem> NewFilter);
	TSubclassOf<UXIUItem> GetFilter() const { return Filter; }
	bool MatchesFilter(const TObjectPtr<UXIUItem> TestItem) const;
	bool MatchesFilterByClass(const TSubclassOf<UXIUItem> TestItem) const;

/*--------------------------------------------------------------------------------------------------------------------*/
	
/*--------------------------------------------------------------------------------------------------------------------*/
	/* Locked */
	
private:
	UPROPERTY()
	bool bLocked;
public:
	void SetLocked(bool NewLock);
	bool IsLocked() const { return bLocked; }

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Generic */

public:
	bool CanInsertItem(const TObjectPtr<UXIUItem> TestItem) const;

/*--------------------------------------------------------------------------------------------------------------------*/

	
	
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * FXIUInventoryList
 */

/** List of inventory items */
USTRUCT(BlueprintType)
struct FXIUInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	FXIUInventoryList()
		: OwnerComponent(nullptr)
	{
	}

	FXIUInventoryList(UXIUInventoryComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent)
    {
    }

private:
	UPROPERTY(NotReplicated)
	TObjectPtr<UXIUInventoryComponent> OwnerComponent;
	
	// Replicated list of items
	UPROPERTY()
	TArray<FXIUInventorySlot> Entries;

/*--------------------------------------------------------------------------------------------------------------------*/
	/* FFastArraySerializer contract */
	
public:
	/* Calls BroadcastChangeMessage, and calls UnBindItemCountChangedDelegate on the items of the removed slots */
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	/* Calls BroadcastChangeMessage, and calls BindItemCountChangedDelegate on the items of the added slots */
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	/* Calls BroadcastChangeMessage, and, if item changed, calls BindItemCountChangedDelegate on the new items, and UnBind on the old one */
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FXIUInventorySlot, FXIUInventoryList>(Entries, DeltaParms, *this);
	}

/*--------------------------------------------------------------------------------------------------------------------*/
	

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Helpers */
	
public:
	void BroadcastChangeMessage(const FXIUInventorySlot& Entry, const int32 OldCount, const int32 NewCount, const bool bItemChanged) const;
private:
	bool CanManipulateInventory() const;

public:
	int GetSize() const { return Entries.Num(); }
	const TArray<FXIUInventorySlot>& GetInventory() const { return Entries; };

/*--------------------------------------------------------------------------------------------------------------------*/
	
/*--------------------------------------------------------------------------------------------------------------------*/
	/* Slots Management */
	
public:
	void InitInventory(int Size);
	
public:
	/** Add a default item
	 * @param ItemDefault: item to add
	 * @param AddedItems: pointers to added items
	 * @return Count of this item which was not added */
	int AddItemDefault(FXIUItemDefault ItemDefault, TArray<TObjectPtr<UXIUItem>>& AddedItems);
	/** Add an item
	 * @param Item: item to add (the count is modified when count is added to existing matching item, and, if
	 *				bDuplicate is true, the count is set to zero if a slot gets filled with duplicate item)
	 * @param bDuplicate: set to true if the Item was not created using this component
	 * @param AddedItem: pointer to added item
	 * @return Count of this item which was not added */
	int AddItem(TObjectPtr<UXIUItem> Item, bool bDuplicate, TObjectPtr<UXIUItem>& AddedItem);

	/** Set item in slot
	 ** @param SlotIndex: Slot to use
	 * @param Item: item to add
	 * @param bDuplicate: set to true if the Item was not created using this component
	 * @param AddedItem: pointer to added item
	 * @param OldItem: pointer to item that was previously in the slot
	 * @return true if item got set */
	bool SetItemAtSlot(int SlotIndex, TObjectPtr<UXIUItem> Item, bool bDuplicate, TObjectPtr<UXIUItem>& AddedItem, TObjectPtr<UXIUItem>& OldItem);
	/** Get item in slot (Already checks IsEmpty on item)
	 * @return pointer to item at index */
	TObjectPtr<UXIUItem> GetItemAtSlot(const int SlotIndex);
	/** Remove item at slot
	 * @return pointer to removed Item */
	TObjectPtr<UXIUItem> RemoveItemAtSlot(int SlotIndex);

	/** @return true if any item was found (Already checks IsEmpty on items) */
	bool GetItemsByClass(const TSubclassOf<UXIUItem> ItemClass, TArray<TObjectPtr<UXIUItem>>& FoundItems);
	/** @return Count actually consumed */
	int ConsumeItemByClass(const TSubclassOf<UXIUItem> ItemClass, const int Count);
	
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Slot Item registration */

public:
	void RegisterSlotChange(const FXIUInventorySlot& Slot, const int32 OldCount, const int32 NewCount, const bool bItemChanged, const TObjectPtr<UXIUItem> OldItem = nullptr);
	
/*--------------------------------------------------------------------------------------------------------------------*/
	

	
};

template<>
struct TStructOpsTypeTraits<FXIUInventoryList> : public TStructOpsTypeTraitsBase2<FXIUInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FXIUInventoryInitializedSignature);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UXIUInventoryComponent
	 */


/**
 * Manages an inventory of fixed size.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class XYLOINVENTORYUTIL_API UXIUInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UXIUInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UActorComponent Interface
	 */

protected:
	virtual void BeginPlay() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * Inventory
	 */

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Delegates */
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_InventoryInitialized)
	bool bInventoryInitialized = false;
	UFUNCTION()
	void OnRep_InventoryInitialized();
public:
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FXIUInventoryInitializedSignature InventoryInitializedDelegate;

public:
	/** PROBABLY SPAGHETTI CODE WARNING!!!
	 * to make this delegate fire both on client and server, some trickery has been used.
	 * SERVER SIDE: every time Slot.SetItem or Slot.Clear is used, or a slot is added/removed,
	 *				I call Inventory.RegisterSlotChange(...)
	 * CLIENT SIDE: using the three functions provided by FFastArraySerializer to track replication changes i call
	 *				Inventory.RegisterSlotChange(...).
	 *				note that as OldItem we use Slot.GetItem() in Remove and Slot.LastObservedItem.Get() in Change, this is
	 *				because we always want to unbind, even if count is zero (and GetItemSafe would not return empty items)
	 * ITEM COUNT: to trigger on item count change, we bind OnItemCountChanged to Item->ItemCountChangedDelegate (Happens in
	 *			   Bind and Unbind functions mentioned above). The bound function is responsible for calling
	 *			   Inventory.RegisterSlotChange(...).
	 *			   Item count is set to zero on client inside OnDestroyedFromReplication, and on server in DestroyObject
	 */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FXIUInventoryChangedSignature InventoryChangedDelegate;
private:
	/* Calls Inventory.BroadcastChangeMessage
	 * Manages the unbinding from item count change delegate in case the item count reaches zero */
	UFUNCTION()
	void OnItemCountChanged(const FXIUItemCountChangeMessage& Change);
public:
	void BindItemCountChangedDelegate(const TObjectPtr<UXIUItem> InItem);
	void UnBindItemCountChangedDelegate(const TObjectPtr<UXIUItem> InItem);

/*--------------------------------------------------------------------------------------------------------------------*/

	
private:
 	UPROPERTY(Replicated)
 	FXIUInventoryList Inventory;
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int InventorySize;
private:
	/** function to set up the slots. should not be used to add items, since it is called before AddDefaultItems
	 * Override in child class to implement. */
	virtual void ApplySettingsToSlots();

private:
	UPROPERTY(EditAnywhere)
	TArray<FXIUItemDefault> DefaultItems;
public:
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	void AddDefaultItems();
	UFUNCTION(Server, Reliable, Category= "Inventory")
	void ServerAddDefaultItems();

	
public:
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	void PrintItems();
	
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	void AddItemDefault(const FXIUItemDefault ItemDefault);

	/** duplicates this item and adds as much count as possible from this duplicate.
	 * The function already modifies the count of the Item passed as parameter to account for
	 * the count actually transferred to this inventory */
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	void AddItem(UXIUItem* Item);

	/** transfer as much count as possible of the item at this slot (internally uses AddItem on OtherInventory) */
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	void TransferItemFromSlot(int SlotIndex, UXIUInventoryComponent* OtherInventory);

	/** Gets first item in the inventory (not necessarily first slot)
	 * (Already checks IsEmpty on items) */
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	UXIUItem* GetFirstItem();

	/** Check if you can insert any count of this item in inventory */
	bool CanInsertItem(UXIUItem* Item) const;

	
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * SubObjects Replication
	 */
	
public:
	/** Register a Replicated UObjects to replicate
	 * @param ObjectToRegister The Replicated UObject to register. */
	virtual bool RegisterReplicatedObject(UPARAM(DisplayName = "Replicated UObject") UObject* ObjectToRegister);

	/** Unregister a Replicated UObject from replication
	 * @param ObjectToUnregister The Replicated UObject to unregister. */
	virtual bool UnregisterReplicatedObject(UPARAM(DisplayName = "Replicated UObject") UObject* ObjectToUnregister);


private:
	//All the currently replicated objects
	UPROPERTY()
	TArray<UObject*> ReplicatedObjects;
public:
	//Get all the replicated objects that are registered on this component.
	virtual TArray<UObject*> GetRegisteredReplicatedObjects() { return ReplicatedObjects; }
	
};
