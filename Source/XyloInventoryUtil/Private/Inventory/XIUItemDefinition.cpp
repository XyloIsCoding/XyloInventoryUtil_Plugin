// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItemDefinition.h"

#include "Inventory/XIUItem.h"
#include "Inventory/XIUItemFragment.h"

const UXIUItem* UXIUItemDefinition::GetItem()
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("Instantiating Item in data asset"))
		Item = NewObject<UXIUItem>(GetOuter(), ItemClass);
	}
	return Item;
}

UXIUItemFragment* UXIUItemDefinition::GetDefaultFragment(const TSubclassOf<UXIUItemFragment> FragmentClass)
{
	UXIUItemFragment* FoundFragment = nullptr;

	if (UClass* TargetClass = FragmentClass.Get())
	{
		for (UXIUItemFragment* DefaultFragment : DefaultFragments)
		{
			if (DefaultFragment && DefaultFragment->IsA(TargetClass))
			{
				FoundFragment = DefaultFragment;
				break;
			}
		}
	}

	return FoundFragment;
}
