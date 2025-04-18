// Copyright XyloIsCoding 2024

#pragma once

#include "CoreMinimal.h"
#include "XIUInventoryChangeMessage.h"
#include "Inventory/Item/XIUItem.h"
#include "XROUObjectReplicatorComponent.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "XIUInventoryComponent.generated.h"


class AXIUItemActor;
struct FXIUInventoryList;
class UXIUInventoryComponent;

USTRUCT(BlueprintType)
struct FXIUInventorySlotSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UXIUItem> Filter;

	UPROPERTY(BlueprintReadWrite)
	bool bLocked = false;
};

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
	{
	}

public:
	FString GetDebugString() const;

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Index */
	
private:
	UPROPERTY()
	int32 Index = -1;
public:
	int32 GetIndex() const { return Index; }

/*--------------------------------------------------------------------------------------------------------------------*/
	
/*--------------------------------------------------------------------------------------------------------------------*/
	/* Item */
	
private:
	UPROPERTY()
	TObjectPtr<UXIUItem> Item = nullptr;
public:
	/** @return true if item got modified */
	bool Clear(UXIUItem*& OldItem);
	/** @return true if item got modified */
	bool SetItem(UXIUItem* NewItem, UXIUItem*& OldItem);
	/** does NOT check for IsEmpty on the item
	 * @return item of this slot. */
	UXIUItem* GetItem() const;
	/** DOES check for IsEmpty on the item
	 * @return item of this slot. */
	UXIUItem* GetItemSafe() const;
	bool IsEmpty() const;

	int32 GetItemCountSafe() const;

	UPROPERTY(NotReplicated)
	TSoftObjectPtr<UXIUItem> LastObservedItem = nullptr;
	UPROPERTY(NotReplicated)
	int32 LastObservedCount = INDEX_NONE;
	
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Filter */
	
private:
	UPROPERTY()
	TSubclassOf<UXIUItem> Filter = nullptr;
public:
	void SetFilter(const TSubclassOf<UXIUItem> NewFilter);
	TSubclassOf<UXIUItem> GetFilter() const { return Filter; }
	bool MatchesFilter(const UXIUItem* TestItem) const;
	bool MatchesFilterByClass(const TSubclassOf<UXIUItem> TestItem) const;

/*--------------------------------------------------------------------------------------------------------------------*/
	
/*--------------------------------------------------------------------------------------------------------------------*/
	/* Locked */
	
private:
	UPROPERTY()
	bool bLocked = false;
public:
	void SetLocked(bool NewLock);
	bool IsLocked() const { return bLocked; }

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Generic */

public:
	bool CanInsertItem(UXIUItem* TestItem) const;
	void ApplySettings(const FXIUInventorySlotSettings& SlotSettings);
	
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
	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
	/* Calls BroadcastChangeMessage, and calls BindItemCountChangedDelegate on the items of the added slots */
	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	/* Calls BroadcastChangeMessage, and, if item changed, calls BindItemCountChangedDelegate on the new items, and UnBind on the old one */
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FXIUInventorySlot, FXIUInventoryList>(Entries, DeltaParms, *this);
	}

/*--------------------------------------------------------------------------------------------------------------------*/
	

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Helpers */
	
public:
	void BroadcastChangeMessage(const FXIUInventorySlot& Entry, const int32 OldCount, const int32 NewCount, UXIUItem* OldItem) const;
private:
	bool CanManipulateInventory() const;

public:
	int32 GetSize() const { return Entries.Num(); }
	const TArray<FXIUInventorySlot>& GetInventory() const { return Entries; };

/*--------------------------------------------------------------------------------------------------------------------*/
	
/*--------------------------------------------------------------------------------------------------------------------*/
	/* Slots Management */
	
public:
	void InitInventory(int32 Size);
	void AddSlot(const FXIUInventorySlotSettings& SlotSettings);
	
public:
	/** Add a default item
	 * @param ItemDefault: item to add
	 * @param AddedItems: pointers to added items
	 * @return Count of this item which was not added */
	int32 AddItemDefault(FXIUItemDefault ItemDefault, TArray<UXIUItem*>& AddedItems);
	/** Add an item
	 * @param Item: item to add (count is decreased to match the amount that was added to inventory, unless
	 *				bModifyItemCount is true)
	 * @param CountOverride
	 * @param bDuplicate: set to true if the Item was not created using this component
	 * @param bModifyItemCount: modify count of item passed as input
	 * @param AddedItem: pointer to added item
	 * @return Count of this item which was not added */
	int32 AddItem(UXIUItem* Item, int32 CountOverride, bool bDuplicate, bool bModifyItemCount, UXIUItem*& AddedItem);

	/** Set item in slot
	 ** @param SlotIndex: Slot to use
	 * @param Item: item to add
	 * @param bDuplicate: set to true if the Item was not created using this component
	 * @param AddedItem: pointer to added item
	 * @param OldItem: pointer to item that was previously in the slot
	 * @return true if item got set */
	bool SetItemAtSlot(int32 SlotIndex, UXIUItem* Item, bool bDuplicate, UXIUItem*& AddedItem, UXIUItem*& OldItem);
	/** Get item in slot (Already checks IsEmpty on item)
	 * @return pointer to item at index */
	UXIUItem* GetItemAtSlot(const int32 SlotIndex);
	/** Remove item at slot
	 * @return pointer to removed Item */
	UXIUItem* RemoveItemAtSlot(int32 SlotIndex);

	/** @return true if any item was found (Already checks IsEmpty on items) */
	bool GetItemsByClass(const TSubclassOf<UXIUItem> ItemClass, TArray<UXIUItem*>& FoundItems);
	/** @return Count actually consumed */
	int32 ConsumeItemByDefinition(const UXIUItemDefinition* ItemDefinition, const int32 Count);
	
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Slot Item registration */

public:
	/**
	 * 
	 * @param Slot: reference to modified slot
	 * @param OldCount: old count of item in slot
	 * @param NewCount: new count of item in slot
	 * @param bRegisterItemChange: if true stops replicating old item and unbind delegate, and start replicating new item and bind delegate
	 * @param OldItem: old item that was in slot
	 */
	void RegisterSlotChange(const FXIUInventorySlot& Slot, const int32 OldCount, const int32 NewCount, const bool bRegisterItemChange, UXIUItem* OldItem = nullptr);
	
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
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class XYLOINVENTORYUTIL_API UXIUInventoryComponent : public UXROUObjectReplicatorComponent
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

public:
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FXIUInventoryInitializedSignature InventoryInitializedDelegate;
protected:
	void SetInventoryInitialized(bool bInitialized);
	UFUNCTION()
	void OnRep_InventoryInitialized();
	UFUNCTION(BlueprintImplementableEvent, Category= "Inventory", DisplayName = "OnInventoryInitialized")
	void BP_OnInventoryInitialized();
private:
	UPROPERTY(ReplicatedUsing = OnRep_InventoryInitialized)
	bool bInventoryInitialized = false;

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
	virtual void BroadcastInventoryChanged(const FXIUInventorySlotChangeMessage& Message);
protected:
	UFUNCTION(BlueprintImplementableEvent, Category= "Inventory", DisplayName = "OnInventoryChanged")
	void BP_OnInventoryChanged();

public:
	void BindItemCountChangedDelegate(UXIUItem* InItem);
	void UnBindItemCountChangedDelegate(UXIUItem* InItem);
private:
	/** Calls Inventory.BroadcastChangeMessage
	 * Manages the unbinding from item count change delegate in case the item count reaches zero */
	UFUNCTION()
	void OnItemCountChanged(const FXIUItemCountChangeMessage& Change);

private:
	/** Calls Inventory.BroadcastChangeMessage */
	UFUNCTION()
	void OnItemInitialized(UXIUItem* InItem);
public:
	void BindItemInitializedDelegate(UXIUItem* InItem);
	void UnBindItemInitializedDelegate(UXIUItem* InItem);
	
	
/*--------------------------------------------------------------------------------------------------------------------*/

public:
	/** Only use this delegate to set up slots. Do not add items here, since AddDefaultItems has not been called yet.
	 * Use InventoryInitializedDelegate to add or manipulate items. */
	UPROPERTY(BlueprintAssignable, Category= "Inventory")
	FXIUInventoryManualInitializationSignature ManualInitializationDelegate;
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	virtual void AddSlot(const FXIUInventorySlotSettings& SlotSettings);
protected:
	/** function to set up the slots. should not be used to add items, since it is called before AddDefaultItems.
	 * Override in child class to implement. */
	virtual void ManualInitialization();
	UFUNCTION(BlueprintImplementableEvent, Category= "Inventory", DisplayName = "OnManualInitialization")
	void BP_OnManualInitialization();
private:
 	UPROPERTY(Replicated)
 	FXIUInventoryList Inventory;
	/** Size of the inventory if bManualInitialization is false */
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 InventorySize;
	UPROPERTY(EditAnywhere, Category = "Inventory")
	bool bManualInitialization;

public:
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	void InputAddDefaultItems();
	UFUNCTION(Server, Reliable, Category= "Inventory")
	void ServerAddDefaultItemsRPC();
	void AddDefaultItems();
private:
	UPROPERTY(EditAnywhere, Category= "Inventory")
	TArray<FXIUItemDefault> DefaultItems;
	
public:
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	void PrintItems();
	
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	void AddItemDefault(const FXIUItemDefault ItemDefault);

	/** duplicates this item and adds as much count as possible from this duplicate.
	 * The function already modifies the count of the Item passed as parameter to account for
	 * the count actually transferred to this inventory */
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	void AddItem(UXIUItem* Item, int32 CountOverride = -1);
	/** Like AddItem, but does not modify the count of Item parameter */
	void AddItemNoModify(UXIUItem* Item, int32 CountOverride = -1);

	/** duplicates this item and sets it in slot (replacing old item if present)
	 * does not modify in any way the item passed as parameter
	 * @return: true if successful */
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	bool SetItemAtSlot(const int32 SlotIndex, UXIUItem* Item);

	/** transfer as much count as possible of the item at this slot (internally uses AddItem on OtherInventory) */
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	void TransferItemFromSlot(int32 SlotIndex, UXIUInventoryComponent* OtherInventory);

	/** drop the item at this slot by spawning a XIUItemActor
	 * @param DropTransform: transform used for deferred spawn
	 * @param SlotIndex: index of the slot to drop the item from
	 * @param Count: count to drop of that item (if -1 drops all)
	 * @param bFinishSpawning: if true, spawns the dropped item actor
	 * @return pointer to the item actor. FinishSpawning must be called
	 */
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	AActor* DropItemAtSlot(const FTransform& DropTransform, const int32 SlotIndex, const int32 Count = -1, const bool bFinishSpawning = true);

	/** @return number of items actually consumed */
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	int32 ConsumeItemsByDefinition(UXIUItemDefinition* ItemDefinition, const int32 Count);
	
	/** Gets first item in the inventory (not necessarily first slot)
	 * (Already checks IsEmpty on items) */
	UFUNCTION(BlueprintCallable, Category= "Inventory")
	UXIUItem* GetFirstItem();

	UFUNCTION(BlueprintCallable, Category= "Inventory")
	int32 CountItemsByDefinition(UXIUItemDefinition* ItemDefinition);

	/** Check if you can insert any count of this item in inventory */
	bool CanInsertItem(UXIUItem* Item) const;

	UFUNCTION(BlueprintCallable, Category= "Inventory")
	UXIUItem* GetItemAtSlot(const int32 SlotIndex);
	
	
};

