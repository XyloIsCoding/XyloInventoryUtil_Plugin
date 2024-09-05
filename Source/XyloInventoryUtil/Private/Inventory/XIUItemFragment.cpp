// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItemFragment.h"


void FXIUFragments::AddFragment(UXIUItemFragment* Fragment)
{
	Fragments.Add(Fragment);
}

void FXIUFragments::RemoveFragment(TSubclassOf<UXIUItemFragment> FragmentClass)
{
	if (FragmentClass != nullptr)
	{
		for (auto FragmentIt = Fragments.CreateIterator(); FragmentIt; ++FragmentIt)
		{
			UXIUItemFragment* Fragment = *FragmentIt;
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				FragmentIt.RemoveCurrent();
			}
		}
	}
}

UXIUItemFragment* FXIUFragments::GetFragment(TSubclassOf<UXIUItemFragment> FragmentClass)
{
	if (FragmentClass != nullptr)
	{
		for (UXIUItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}

void UXIUItemFragment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
}
