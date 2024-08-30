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
struct FXIUInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

private:
	friend FXIUInventoryList;
	friend UXIUInventoryComponent;

public:
	FXIUInventoryEntry()
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
		return FFastArraySerializer::FastArrayDeltaSerialize<FXIUInventoryEntry, FXIUInventoryList>(Entries, DeltaParms, *this);
	}

	TArray<UXIUItemStack*> GetAllItems() const;

	UXIUItemStack* AddEntry(TSubclassOf<UXIUItem> ItemClass, int32 StackCount);
	void AddEntry(UXIUItemStack* Stack);

	void RemoveEntry(UXIUItemStack* Stack);

private:
	void BroadcastChangeMessage(FXIUInventoryEntry& Entry, int32 OldCount, int32 NewCount);

private:
	// Replicated list of items
	UPROPERTY()
	TArray<FXIUInventoryEntry> Entries;

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
	UXIUInventoryComponent();

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
 	UPROPERTY(EditDefaultsOnly)
 	FXIUInventoryList Inventory;
};
