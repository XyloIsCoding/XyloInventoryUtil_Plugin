// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Fragment/XIUDurabilityFragment.h"

bool UXIUDurabilityFragment::Matches(UXIUItemFragment* Fragment) const
{
	if (!Super::Matches(Fragment)) return false;
	
	if (const UXIUDurabilityFragment* DurabilityFragment = Cast<UXIUDurabilityFragment>(Fragment))
	{
		if (Durability == DurabilityFragment->Durability && MaxDurability == DurabilityFragment->Durability)
		{
			return true;
		}
	}
	return false;
}
