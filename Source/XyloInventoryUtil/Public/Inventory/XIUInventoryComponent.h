// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XIUItem.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "XIUInventoryComponent.generated.h"


class UXIUItemDefinition;
struct FXIUInventoryList;
class UXIUInventoryComponent;
class UXIUItemStack;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * FXIUInventorySlot Interface
 */

/** A single entry in an inventory */
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
	/* Stack */
	
private:
	UPROPERTY()
	TObjectPtr<UXIUItemStack> Stack = nullptr;
public:
	/** @return true if stack got modified */
	bool Clear(TObjectPtr<UXIUItemStack>& OldStack);
	/** @return true if stack got modified */
	bool SetItemStack(TObjectPtr<UXIUItemStack> NewStack, TObjectPtr<UXIUItemStack>& OldStack);
	UXIUItemStack* GetItemStack() const { return Stack; }
	bool IsEmpty() const;
	
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Filter */
	
private:
	UPROPERTY()
	TSubclassOf<UXIUItem> Filter;
public:
	void SetFilter(TSubclassOf<UXIUItem> NewFilter);
	TSubclassOf<UXIUItem> GetFilter() const { return Filter; }
	bool MatchesFilter(const TObjectPtr<UXIUItemStack> ItemStack) const;

/*--------------------------------------------------------------------------------------------------------------------*/
	
/*--------------------------------------------------------------------------------------------------------------------*/
	/* Locked */
	
private:
	UPROPERTY()
	bool bLocked;
public:
	void SetLocked(bool NewLock);
	bool GetLocked() const { return bLocked; }

/*--------------------------------------------------------------------------------------------------------------------*/
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * FXIUInventoryList Interface
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


/*--------------------------------------------------------------------------------------------------------------------*/
	/* FFastArraySerializer contract */
	
public:
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FXIUInventorySlot, FXIUInventoryList>(Entries, DeltaParms, *this);
	}

/*--------------------------------------------------------------------------------------------------------------------*/
	
	
private:
	void BroadcastChangeMessage(FXIUInventorySlot& Entry, int32 OldCount, int32 NewCount);

private:
	bool CanManipulateInventory() const;
	
public:
	void InitInventory(int Size);

public:
	int GetSize() const { return Entries.Num(); }

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Slots Management */
	
private:
	// Replicated list of items
	UPROPERTY()
	TArray<FXIUInventorySlot> Entries;
	
public:
	const TArray<FXIUInventorySlot>& GetInventory() const;
	UXIUItemStack* GetStackAtSlot(int SlotIndex);

public:
	/** Add count of an item to the inventory. increases count of existing stacks if it can,
	 * else tries to add a new stack with defaulted fragments
	 * @return Count added */
	 int AddItem(UXIUItemDefinition* ItemDefinition, int32 Count, TArray<UXIUItemStack*> AddedStacks);
	/** Tries to add an item stack to the inventory, first by increasing count of existing matching stacks, then by adding
	 * the stack itself with the remaining count
	 * @return Count added */
	int AddItemStack(UXIUItemStack* ItemStack, bool bUpdateOwningInventory = true);
private:
	/** Tries to add an item stack to the first empty slot which allows the stack to be inserted.
	 * @return true if the stack got added */
	bool AddItemStackInEmptySlot(UXIUItemStack* ItemStack, bool bUpdateOwningInventory);

public:
	/** Removes the stack from the inventory
	 * @return true if the stack got removed*/
	bool RemoveItemStack(UXIUItemStack* ItemStack);
	/** Removes count from an item stack of this inventory
	 * @return Count still to remove */
	int RemoveCountFromItemStack(UXIUItemStack* ItemStack, int32 Count);
	/** Removes count of an item in this inventory
	 * @return Count still to remove */
	int ConsumeItem(UXIUItemDefinition* ItemDefinition, int32 Count);

public:
	/** Set a stack in a slot
	 * @param ItemStack: the stack to set
	 * @param SlotIndex: index of the slot
	 * @param OldStack: stack that was in the slot
	 * @param bUpdateOwningInventory: if the owning inventory of ItemStack should be updated
	 * @return true if stack got set */
	bool SetItemStackInSlot(UXIUItemStack* ItemStack, int SlotIndex, TObjectPtr<UXIUItemStack>& OldStack, bool bUpdateOwningInventory);
	/** Extract a stack from a slot
	 * @param SlotIndex: index of the slot
	 * @param ExtractedStack: stack that was in the slot
	 * @return true if stack got extracted */
	bool ExtractItemStackFromSlot(int SlotIndex, TObjectPtr<UXIUItemStack>& ExtractedStack);

/*--------------------------------------------------------------------------------------------------------------------*/
	
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * Inventory
	 */

private:
 	UPROPERTY(Replicated)
 	FXIUInventoryList Inventory;
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int InventorySize;
private:
	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UXIUItemDefinition>> DefaultItems;

	UPROPERTY(EditAnywhere)
	TArray<UXIUItem*> DefaultItems2;

	UPROPERTY(EditAnywhere)
	UXIUItem* DefaultItems3;

public:
	UFUNCTION(BlueprintCallable, Category=Inventory)
	void AddDefaultItems();
	UFUNCTION(Server, Reliable, Category=Inventory)
	void ServerAddDefaultItems();

	UFUNCTION(BlueprintCallable, Category=Inventory)
	void PrintItems();

	UFUNCTION(BlueprintCallable, Category=Inventory)
	UXIUItemStack* GetStackAtSlot(int32 SlotIndex);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	TArray<UXIUItemStack*> AddItem(UXIUItemDefinition* ItemDefinition, const int32 Count = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void AddItemStack(UXIUItemStack* ItemStack);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	bool ConsumeItem(UXIUItemDefinition* ItemDefinition, int32 Count);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * SubObjects Replication
	 */
	
public:
	
	/**
	 * Register a Replicated UObjects to replicate
	 * 
	 * @param ObjectToRegister The Replicated UObject to register.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Object Replication", DisplayName = "Register Replicated UObject")
	virtual bool RegisterReplicatedObject(UPARAM(DisplayName = "Replicated UObject") UObject* ObjectToRegister);

	/**
	 * Unregister a Replicated UObject from replication
	 * 
	 * @param ObjectToUnregister The Replicated UObject to unregister.
	 * @param bDestroyObject If the UObject Replication Manager should mark the Replicated UObject as garbage for the garbage collector, after it has unregistered the Replicated UObject.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Object Replication", DisplayName = "Unregister Replicated UObject")
	virtual bool UnregisterReplicatedObject(UPARAM(DisplayName = "Replicated UObject") UObject* ObjectToUnregister);

	//Get all the replicated objects that are registered on this manager.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Object Replication", DisplayName = "Get Registered Replicated UObjects")
	virtual TArray<UObject*> GetRegisteredReplicatedObjects()
	{
		return ReplicatedObjects;
	};

protected:
	
	//All the currently replicated objects
	UPROPERTY()
	TArray<UObject*> ReplicatedObjects;
	
};
