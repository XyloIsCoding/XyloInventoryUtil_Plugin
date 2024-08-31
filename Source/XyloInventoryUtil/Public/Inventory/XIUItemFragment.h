// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "XIUItemFragment.generated.h"


class UXIUItemFragment;
class UXIUItemStack;


USTRUCT(BlueprintType)
struct FXIUFragments
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Item")
	TMap<FGameplayTag, TObjectPtr<UXIUItemFragment>> Fragments;
	
public:
	void AddFragment(UXIUItemFragment* Fragment);
	void RemoveFragment(FGameplayTag FragmentTag);

	UXIUItemFragment* GetFragment(FGameplayTag FragmentTag);
};


/*
 * Represents a fragment of an item definition
 */

UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class XYLOINVENTORYUTIL_API UXIUItemFragment : public UObject
{
	GENERATED_BODY()

private:
	FGameplayTag FragmentTag;
public:
	FORCEINLINE FGameplayTag GetFragmentTag() const { return FragmentTag; }
	
public:
	virtual void OnInstanceCreated(UXIUItemStack* Instance) const {}
};
