// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItemFragment.h"


void FXIUFragments::AddFragment(UXIUItemFragment* Fragment)
{
	Fragments.Add(Fragment->GetFragmentTag(), Fragment);
}

void FXIUFragments::RemoveFragment(FGameplayTag FragmentTag)
{
	Fragments.Remove(FragmentTag);
}

UXIUItemFragment* FXIUFragments::GetFragment(FGameplayTag FragmentTag)
{
	return *Fragments.Find(FragmentTag);
}
