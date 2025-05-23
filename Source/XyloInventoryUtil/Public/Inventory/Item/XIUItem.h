// Copyright XyloIsCoding 2024

#pragma once

#include "CoreMinimal.h"
#include "XROUReplicatedObject.h"
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

	FXIUItemCountChangeMessage(UXIUItem* Item, int32 OldCount)
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
	
	FXIUItemDefault(UXIUItemDefinition* ItemDefinition, int32 Count)
		: ItemDefinition(ItemDefinition),
		  Count(Count)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UXIUItemDefinition> ItemDefinition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count;


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
 * only use UXIUInventoryUtilLibrary::MakeItemFromDefault to create an Item object
 * if IsEmpty() || !IsItemInitialized(), do not use this item. consider it as nullptr.
 * you can use the static function IsItemAvailable(Item) to make sure you can use the item.
 */ 
UCLASS(Blueprintable, BlueprintType, Abstract)
class XYLOINVENTORYUTIL_API UXIUItem : public UXROUReplicatedObject
{
	GENERATED_BODY()

public:
	UXIUItem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UObject Interface
	 */

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UXROUReplicatedObject Interface
	 */

public:
	virtual void OnDestroyed() override;

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
	UFUNCTION(BlueprintCallable)
	static bool IsItemInitialized(UXIUItem* Item);

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
	/** Should only be called if bIsActive == true
	 * <p> Note: currently called only in OnDestroyed */
	virtual void DestroyActiveState();
private:
	bool bItemInitialized = false;
	bool bIsActive = false;
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
	int32 GetMaxCount() const;
	/** @return item count */
	UFUNCTION(BlueprintCallable)
	int32 GetCount() const;
	/** Set the count of the item */
	UFUNCTION(BlueprintCallable)
	void SetCount(int32 NewCount);
	/** @return count added */
	UFUNCTION(BlueprintCallable)
	int32 ModifyCount(const int32 AddCount);
protected:
	UFUNCTION()
	virtual void OnRep_Count(int32 OldCount);
private:
	UPROPERTY(ReplicatedUsing = OnRep_Count)
	int32 Count = -1;
	int32 LastCount = -1;

/*--------------------------------------------------------------------------------------------------------------------*/
	
};
