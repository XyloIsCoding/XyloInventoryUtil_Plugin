// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItemFragment.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXIUItemFragment
 */

void UXIUItemFragment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
}

UXIUItemFragment* UXIUItemFragment::Duplicate(UObject* Outer) const
{
	if (UXIUItemFragment* DuplicateFragment = DuplicateObject<UXIUItemFragment>(this, Outer))
	{
		// clear this two flags to allow client to update the outer of the duplicated fragment
		DuplicateFragment->ClearFlags(EObjectFlags::RF_WasLoaded);
		DuplicateFragment->ClearFlags(EObjectFlags::RF_LoadCompleted);
		return DuplicateFragment;
	}
	return nullptr;
}

bool UXIUItemFragment::Matches(UXIUItemFragment* Fragment) const
{
	return true;
}
