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
	TArray<TObjectPtr<UXIUItemFragment>> Fragments;
	
public:
	void AddFragment(UXIUItemFragment* Fragment);
	void RemoveFragment(TSubclassOf<UXIUItemFragment> FragmentClass);

	UXIUItemFragment* GetFragment(TSubclassOf<UXIUItemFragment> FragmentClass);
	TArray<TObjectPtr<UXIUItemFragment>> GetAllFragments() const { return Fragments; }
};


/*
 * Represents a fragment of an item definition
 */

UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class XYLOINVENTORYUTIL_API UXIUItemFragment : public UObject
{
	GENERATED_BODY()

public:
	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UObject interface
	
public:
	virtual void OnInstanceCreated(UXIUItemStack* Instance) const {}
};
