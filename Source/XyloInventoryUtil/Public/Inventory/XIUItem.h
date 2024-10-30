// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Interface_ActorSubobject.h"
#include "UObject/Object.h"
#include "XIUItem.generated.h"

class UXIUItemDefinition;
class UXIUItem;

USTRUCT(BlueprintType)
struct FXIUItemCountChangeMessage
{
	GENERATED_BODY()

	FXIUItemCountChangeMessage()
	{
		
	}

	FXIUItemCountChangeMessage(TObjectPtr<UXIUItem> Item, int32 OldCount)
		:  Item(Item),
		   OldCount(OldCount)
	{
		
	}
	
	UPROPERTY(BlueprintReadOnly, Category=Item)
	TObjectPtr<UXIUItem> Item = nullptr;

	UPROPERTY(BlueprintReadOnly, Category=Item)
	int32 OldCount = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FXIUItemInitializedSignature, UXIUItem*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FXIUItemCountChangedSignature, const FXIUItemCountChangeMessage&, Change);



USTRUCT(BlueprintType)
struct FXIUItemDefault
{
	GENERATED_BODY()

	FXIUItemDefault()
		: ItemDefinition(nullptr),
		  Count(0)
	{
	}
	
	FXIUItemDefault(UXIUItemDefinition* ItemDefinition, int Count)
		: ItemDefinition(ItemDefinition),
		  Count(Count)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UXIUItemDefinition> ItemDefinition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Count;


	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << ItemDefinition;
		Ar << Count;
		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FXIUItemDefault> : public TStructOpsTypeTraitsBase2<FXIUItemDefault>
{
	enum
	{
		WithNetSerializer = true
	};
};




/**
 * only use UXIUInventoryFunctionLibrary::MakeItemFromDefault to create a Item object
 * if IsEmpty() returns false, do not use this item. consider it as nullptr.
 */ 
UCLASS(Blueprintable, BlueprintType, Abstract)
class XYLOINVENTORYUTIL_API UXIUItem : public UObject, public IInterface_ActorSubobject
{
	GENERATED_BODY()

public:
	UXIUItem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	//Gets the Actor that "owns" this Replicated UObject.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Replicated UObject")
	AActor* GetOwningActor() const;
	
public:
	//Will mark this UObject as garbage and will eventually get cleaned by the garbage collector.
	//Should only execute this on the server.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Replicated UObject", DisplayName = "Destroy Replicated UObject")
	void DestroyObject();
protected:
	//Will get called on the server once the Replicated UObject has been marked as garbage.
	UFUNCTION(BlueprintImplementableEvent, Category = "Replicated UObject", DisplayName = "On Replicated UObject Destroyed")
	void OnDestroyed();
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UObject Interface
	 */
	
public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual UWorld* GetWorld() const override;
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override;
public:
	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker);
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * IInterface_ActorSubobject Interface
	 */

public:
	virtual void OnCreatedFromReplication() override;
	virtual void OnDestroyedFromReplication() override;
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * Item
	 */

public:
	UFUNCTION(BlueprintCallable)
	FString GetItemName() const;

public:
	UFUNCTION(BlueprintCallable)
	bool IsEmpty() const;
	UFUNCTION(BlueprintCallable)
	bool IsFull() const;
	UFUNCTION(BlueprintCallable)
	virtual bool CanStack(UXIUItem* Item);
	UFUNCTION(BlueprintCallable)
	virtual UXIUItem* Duplicate(UObject* Outer);
	UFUNCTION(BlueprintCallable)
	static bool IsItemAvailable(UXIUItem* Item);

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Initialization */

public:
	UPROPERTY(BlueprintAssignable)
	FXIUItemInitializedSignature ItemInitializedDelegate;
	bool IsItemInitialized() const;
	void InitializeItem(const FXIUItemDefault& InItemInitializer);
protected:
	UFUNCTION()
	void OnRep_ItemInitializer();
	void InitializingItem();
	virtual void OnItemInitialized();
private:
	bool bItemInitialized = false;
	UPROPERTY(ReplicatedUsing = OnRep_ItemInitializer)
	FXIUItemDefault ItemInitializer;
	
/*--------------------------------------------------------------------------------------------------------------------*/
	
/*--------------------------------------------------------------------------------------------------------------------*/
	/* ItemDefinition */

public:
	UFUNCTION(BlueprintCallable, Category = "Item")
	UXIUItemDefinition* GetItemDefinition() const { return ItemDefinition; }
protected:
	void SetItemDefinition(UXIUItemDefinition* InItemDefinition);
private:
	UPROPERTY()
	UXIUItemDefinition* ItemDefinition;

/*--------------------------------------------------------------------------------------------------------------------*/
	
/*--------------------------------------------------------------------------------------------------------------------*/
	/* Count */

public:
	UPROPERTY(BlueprintAssignable)
	FXIUItemCountChangedSignature ItemCountChangedDelegate;
public:
	/** @return item count */
	UFUNCTION(BlueprintCallable)
	int GetCount() const;
	/** Set the count of the item */
	UFUNCTION(BlueprintCallable)
	void SetCount(int32 NewCount);
	/** @return count added */
	UFUNCTION(BlueprintCallable)
	int ModifyCount(const int AddCount);
protected:
	UFUNCTION()
	virtual void OnRep_Count(int32 OldCount);
private:
	UPROPERTY(ReplicatedUsing = OnRep_Count)
	int Count = -1;

/*--------------------------------------------------------------------------------------------------------------------*/
	
};
