// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XIUItem.h"
#include "XIUItemFragment.h"
#include "UObject/Object.h"
#include "XIUItemStack.generated.h"

class UXIUCountFragment;
struct FXIUFragments;
class UXIUItemFragment;
class UXIUItem;


/**
 * Item Stack outer is the owner of the inventory component
 */
UCLASS(BlueprintType)
class XYLOINVENTORYUTIL_API UXIUItemStack : public UObject
{
	GENERATED_BODY()

public:
	UXIUItemStack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UObject Interface
	 */

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * ItemStack
	 */

private:
	TObjectPtr<UXIUInventoryComponent> OwningInventoryComponent;
public:
	/** deals with the replication of the stack and its fragments.
	 * must be called if the stack gets transferred to another inventory component */
	void SetOwningInventoryComponent(UXIUInventoryComponent* InventoryComponent);
	
private:
	UPROPERTY(Replicated)
	const UXIUItem* Item;
public:
	/** Change the item of this stack (resets the fragments) */
	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetItem(const UXIUItem* NewItem);
	UFUNCTION(BlueprintCallable, Category = "Item")
	const UXIUItem* GetItem() const;

private:
	UPROPERTY(Replicated)
	FXIUFragments Fragments;
public:
	TArray<const UXIUItemFragment*> GetAllFragments() const;
	
	UFUNCTION(BlueprintCallable, Category = "Fragments")
	void SetFragment(TSubclassOf<UXIUItemFragment> FragmentClass, UXIUItemFragment* Fragment);



	
	UFUNCTION(BlueprintCallable, Category = "Count Fragment")
	int GetCount();
	/** @return new count */
	UFUNCTION(BlueprintCallable, Category = "Count Fragment")
	int SetCount(int NewCount);
	/** @return count actually added (or removed if negative) */
	UFUNCTION(BlueprintCallable, Category = "Count Fragment")
	int ModifyCount(int AddCount);
	
};
