// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "XIUItemFragment.generated.h"


class UXIUInventoryComponent;
class UXIUItemFragment;
class UXIUItemStack;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXIUItemFragment
 */


/*
 * Represents a fragment of an item definition
 */
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class XYLOINVENTORYUTIL_API UXIUItemFragment : public UObject
{
	GENERATED_BODY()

public:
	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return bReplicated; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UObject interface
	
public:
	virtual void OnInstanceCreated(UXIUItemStack* Instance) const {}

	/** @return Duplicated fragment using specified outer */
	UFUNCTION(BlueprintCallable, Category = "Fragment")
	virtual UXIUItemFragment* Duplicate(UObject* Outer) const;
	
	/** Defines whether two fragments are the same. Should be overridden by child classes */
	UFUNCTION(BlueprintCallable, Category = "Fragment")
	virtual bool Matches(UXIUItemFragment* Fragment) const;
	
private:
    UPROPERTY(EditDefaultsOnly, Category = "Replication")
    bool bReplicated = true;
};
