// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XIUItem.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "XIUInventoryComponent.generated.h"


struct FXIUInventoryList;
class UXIUInventoryComponent;
class UXIUItemStack;


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
	friend UXIUInventoryComponent;

public:
	FXIUInventorySlot()
		: Index(-1),
		  bLocked(false)
	{
	}
	
	FXIUInventorySlot(int SlotIndex)
		: Index(SlotIndex),
		  bLocked(false)
	{
	}

public:
	FString GetDebugString() const;

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Index */
	
private:
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
	void Clear();
	bool SetItemStack(TObjectPtr<UXIUItemStack> NewStack);
	UXIUItemStack* GetItemStack() const { return Stack; }

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Filter */
	
private:
	TSubclassOf<UXIUItem> Filter;
public:
	void SetFilter(TSubclassOf<UXIUItem> NewFilter);
	TSubclassOf<UXIUItem> GetFilter() const { return Filter; }

/*--------------------------------------------------------------------------------------------------------------------*/
	
/*--------------------------------------------------------------------------------------------------------------------*/
	/* Locked */
	
private:
	bool bLocked;
public:
	void SetLocked(bool NewLock);
	bool GetLocked() const { return bLocked; }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * FXIUInventoryList Interface
 */

/** List of inventory items */
USTRUCT(BlueprintType)
struct FXIUInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

private:
	friend UXIUInventoryComponent;

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
	
private:
	void InitInventory(int Size);

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Slots Management */
	
private:
	// Replicated list of items
	UPROPERTY()
	TArray<FXIUInventorySlot> Entries;
	
public:
	TArray<UXIUItemStack*> GetAllItems() const;

public:
	TArray<UXIUItemStack*> AddItem(UXIUItem* Item, int32 Count);
	bool AddItemStack(UXIUItemStack* ItemStack, bool bUpdateOwningInventory = true);
private:
	bool AddItemStackInEmptySlot(UXIUItemStack* ItemStack, bool bUpdateOwningInventory = true);

public:
	/** @return Count still to remove */
	int RemoveCountFromItemStack(UXIUItemStack* ItemStack, int32 Count);
	/** @return Count still to remove */
	int ConsumeItem(UXIUItem* Item, int32 Count);
	void ClearSlot(int SlotIndex);



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
	TArray<TObjectPtr<UXIUItem>> DefaultItems;

public:
	UFUNCTION(BlueprintCallable)
	void AddDefaultItems();
	UFUNCTION(Server, Reliable)
	void ServerAddDefaultItems();
	
	UFUNCTION(BlueprintCallable)
	void PrintItems();
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	bool CanAddItem(TSubclassOf<UXIUItem> Item, int32 Count = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	TArray<UXIUItemStack*> AddItem(UXIUItem* Item, const int32 Count = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void AddItemStack(UXIUItemStack* ItemStack);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	bool ConsumeItem(UXIUItem* Item, int32 Count);
	
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure=false)
	TArray<UXIUItemStack*> GetAllItems() const;


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
