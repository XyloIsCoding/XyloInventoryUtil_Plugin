// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ORReplicatedObject.h"
#include "XIUItemFragment.h"
#include "UObject/Object.h"
#include "XIUItemStack.generated.h"

class UXIUCountFragment;
struct FXIUFragments;
class UXIUItemFragment;
class UXIUItem;
/**
 * 
 */
UCLASS(BlueprintType)
class XYLOINVENTORYUTIL_API UXIUItemStack : public UORReplicatedObject
{
	GENERATED_BODY()

public:
	UXIUItemStack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UObject Interface
	 */

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

	UXIUItemFragment* FindFragmentByClass(TSubclassOf<UXIUItemFragment> FragmentClass);
};
