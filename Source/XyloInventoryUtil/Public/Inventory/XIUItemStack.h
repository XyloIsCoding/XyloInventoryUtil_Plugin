// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XIUItemFragment.h"
#include "UObject/Object.h"
#include "XIUItemStack.generated.h"

struct FXIUFragments;
class UXIUItemFragment;
class UXIUItem;
/**
 * 
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

	/* Fragments replication */
	
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * ItemStack
	 */
	
private:
	UPROPERTY(Replicated)
	const UXIUItem* Item;

protected:
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Item")
	FXIUFragments Fragments;
public:
	UXIUItemFragment* AddFragment(UXIUItemFragment* ItemFragment);
	TArray<TObjectPtr<UXIUItemFragment>> GetAllFragments() const { return Fragments.GetAllFragments(); }

public:
	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetItem(const UXIUItem* NewItem);
	UFUNCTION(BlueprintCallable, Category = "Item")
	const UXIUItem* GetItem() const;
	UFUNCTION(BlueprintCallable, Category = "Item")
	int GetCount();
	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetCount(int NewCount);
	/** @return count actually added */
	UFUNCTION(BlueprintCallable, Category = "Item")
	int AddCount(int AddCount);

	UPROPERTY(Replicated)
	int TestCount;

	UXIUItemFragment* FindFragmentByClass(TSubclassOf<UXIUItemFragment> FragmentClass);
};
