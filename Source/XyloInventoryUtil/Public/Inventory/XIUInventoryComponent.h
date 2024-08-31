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
 * FLyraInventoryEntry Interface
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
	{}
	
public:
	FString GetDebugString() const;

private:
	UPROPERTY()
	TObjectPtr<UXIUItemStack> Stack = nullptr;
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

public:
	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FXIUInventorySlot, FXIUInventoryList>(Entries, DeltaParms, *this);
	}

	TArray<UXIUItemStack*> GetAllItems() const;

	UXIUItemStack* AddEntry(TSubclassOf<UXIUItem> ItemClass, int32 StackCount);
	void AddEntry(UXIUItemStack* Stack);

	void RemoveEntry(UXIUItemStack* Stack);

private:
	void BroadcastChangeMessage(FXIUInventorySlot& Entry, int32 OldCount, int32 NewCount);

private:
	// Replicated list of items
	UPROPERTY()
	TArray<FXIUInventorySlot> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
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
 * UObject Interface
 */

public:
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;

	
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
 	UPROPERTY(EditDefaultsOnly, Replicated)
 	FXIUInventoryList Inventory;

public:
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
};
