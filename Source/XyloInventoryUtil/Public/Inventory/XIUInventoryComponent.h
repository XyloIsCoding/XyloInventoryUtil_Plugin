// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ORManagerComponent.h"
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

	FXIUInventoryList(UActorComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent)
    {
    }

private:
	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;


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
	bool CanManipulateInventory();

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

	UXIUItemStack* AddItem(TSubclassOf<UXIUItem> ItemClass, int32 StackCount);
	void AddItemStack(UXIUItemStack* Stack);
	void RemoveCountFromItemStack(UXIUItemStack* Stack, int32 StackCount);
	void ClearSlot(int SlotIndex);

/*--------------------------------------------------------------------------------------------------------------------*/
	
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class XYLOINVENTORYUTIL_API UXIUInventoryComponent : public UORManagerComponent
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
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UXIUItem>> DefaultItems;

public:
	UFUNCTION(BlueprintCallable)
	void AddDefaultItems();

	UFUNCTION(Server, Reliable)
	void ServerAddDefaultItems();
	
	UFUNCTION(BlueprintCallable)
	void PrintItems();
	


	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	bool CanAddItemDefinition(TSubclassOf<UXIUItem> ItemDef, int32 StackCount = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	UXIUItemStack* AddItemDefinition(TSubclassOf<UXIUItem> ItemDef, int32 StackCount = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void AddItemInstance(UXIUItemStack* ItemInstance);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void RemoveItemInstance(UXIUItemStack* ItemInstance);

	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure=false)
	TArray<UXIUItemStack*> GetAllItems() const;

	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)
	UXIUItemStack* FindFirstItemStackByDefinition(TSubclassOf<UXIUItem> ItemDef) const;

	int32 GetTotalItemCountByDefinition(TSubclassOf<UXIUItem> ItemDef) const;
	bool ConsumeItemsByDefinition(TSubclassOf<UXIUItem> ItemDef, int32 NumToConsume);



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
};
