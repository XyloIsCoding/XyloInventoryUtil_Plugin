// Copyright XyloIsCoding 2024


#include "Inventory/Item/XIUItemDefinition.h"

void UXIUItemFragment::OnInstanceCreated_Implementation(UXIUItem* Item) const
{
}

UXIUItemDefinition::UXIUItemDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxCount = 1;
}

const UXIUItemFragment* UXIUItemDefinition::FindFragmentByClass(const TSubclassOf<UXIUItemFragment> FragmentClass) const
{
	if (FragmentClass != nullptr)
	{
		for (const UXIUItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}
