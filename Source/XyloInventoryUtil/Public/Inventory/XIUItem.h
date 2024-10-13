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
	UXIUItemDefinition* ItemDefinition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Count;
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

private:
	bool bItemInitialized;

private:
	UPROPERTY(Replicated)
	UXIUItemDefinition* ItemDefinition;
public:
	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetItemDefinition(UXIUItemDefinition* InItemDefinition);
	UFUNCTION(BlueprintCallable, Category = "Item")
	UXIUItemDefinition* GetItemDefinition() const { return ItemDefinition; }
protected:
	/** Called by SetItemDefinition */
	virtual void OnItemInitialized();
	
public:
	UPROPERTY(BlueprintAssignable)
	FXIUItemCountChangedSignature ItemCountChangedDelegate;

public:
	UFUNCTION(BlueprintCallable)
	FString GetItemName() const;

private:
	UPROPERTY(ReplicatedUsing = OnRep_Count)
	int Count = -1;
	UFUNCTION()
	void OnRep_Count(int OldCount);
public:
	/** @return item count */
	UFUNCTION(BlueprintCallable)
	int GetCount() const;
	/** Set the count of the item */
	UFUNCTION(BlueprintCallable)
	void SetCount(int NewCount);
	/** @return count added */
	UFUNCTION(BlueprintCallable)
	int ModifyCount(const int AddCount);

public:
	UFUNCTION(BlueprintCallable)
	bool IsEmpty() const;
	UFUNCTION(BlueprintCallable)
	bool IsFull() const;
	UFUNCTION(BlueprintCallable)
	virtual bool CanStack(UXIUItem* Item);
	UFUNCTION(BlueprintCallable)
	virtual UXIUItem* Duplicate(UObject* Outer);
	

};
