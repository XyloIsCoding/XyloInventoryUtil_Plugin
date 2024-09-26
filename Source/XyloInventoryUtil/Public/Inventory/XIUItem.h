// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "XIUItem.generated.h"

class UXIUItem;

USTRUCT(BlueprintType)
struct FXIUItemCountChangeMessage
{
	GENERATED_BODY()

	FXIUItemCountChangeMessage()
	{
		
	}

	FXIUItemCountChangeMessage(TObjectPtr<UXIUItem> Item, int32 NewCount, int32 OldCount)
		:  Item(Item),
		   NewCount(NewCount),
		   OldCount(OldCount)
	{
		
	}

	
	UPROPERTY(BlueprintReadOnly, Category=Item)
	TObjectPtr<UXIUItem> Item = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category=Item)
	int32 NewCount = 0;

	UPROPERTY(BlueprintReadOnly, Category=Item)
	int32 OldCount = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FXIUItemCountChangedSignature, const FXIUItemCountChangeMessage&, Change);




USTRUCT(BlueprintType)
struct FXIUItemDefault
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UXIUItem> ItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Count;
};




/**
 * if IsEmpty() returns false, do not use this item. consider it as nullptr.
 */ 
UCLASS(Blueprintable, BlueprintType, Abstract)
class XYLOINVENTORYUTIL_API UXIUItem : public UObject
{
	GENERATED_BODY()

public:
	UXIUItem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;



public:
	UPROPERTY(BlueprintAssignable)
	FXIUItemCountChangedSignature ItemCountChangedDelegate;
	
private:
	UPROPERTY(EditAnywhere, Category = "Item")
	FString ItemName;
public:
	UFUNCTION(BlueprintCallable)
	FString GetItemName() const;

	
private:
	UPROPERTY(EditAnywhere, Replicated, Category = "Item")
	int MaxCount;
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
