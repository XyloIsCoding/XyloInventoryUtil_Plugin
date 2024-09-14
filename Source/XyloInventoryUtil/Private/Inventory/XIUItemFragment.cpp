// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItemFragment.h"

#include "Inventory/XIUInventoryComponent.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * FXIUDefaultFragments
 */

TArray<UXIUItemFragment*> FXIUDefaultFragments::GetAllFragments() const
{
	TArray<UXIUItemFragment*> Result;
	Result.Reserve(DefaultFragments.Num());
	for (UXIUItemFragment* Fragment : DefaultFragments)
	{
		Result.Add(Fragment);
	}
	return Result;
}

UXIUItemFragment* FXIUDefaultFragments::FindDefaultFragmentByClass(const TSubclassOf<UXIUItemFragment> FragmentClass) const
{
	UXIUItemFragment* FoundFragment = nullptr;

	if (UClass* TargetClass = FragmentClass.Get())
	{
		for (UXIUItemFragment* Fragment : DefaultFragments)
		{
			if (Fragment && Fragment->IsA(TargetClass))
			{
				FoundFragment = Fragment;
				break;
			}
		}
	}

	return FoundFragment;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * FXIUFragments
 */

void FXIUFragments::SetOwningInventoryComponent(TObjectPtr<UXIUInventoryComponent> InventoryComponent)
{
	UXIUInventoryComponent* OldOwningComponent = OwningInventoryComponent;
	OwningInventoryComponent = InventoryComponent;

	for (UXIUItemFragment* Fragment : ChangedFragments)
	{
		if (Fragment)
		{
			// unregister from old inventory component owner
			if (OldOwningComponent) OldOwningComponent->UnregisterReplicatedObject(Fragment);
			
			if (InventoryComponent)
			{
				// rename if necessary and register to new inventory component owner
				if (OldOwningComponent) Fragment->Rename(nullptr, OwningInventoryComponent->GetOwner());
				OwningInventoryComponent->RegisterReplicatedObject(Fragment);
			}
		}
	}
}

UXIUItemFragment* FXIUFragments::DuplicateAndAdd(const UXIUItemFragment* NewFragment)
{
	if (NewFragment)
	{
		if (UXIUItemFragment* DuplicateFragment = DuplicateObject<UXIUItemFragment>(NewFragment, OwningInventoryComponent->GetOwner()))
		{
			// clear this two flags to allow client to update the outer of the duplicated fragment
			DuplicateFragment->ClearFlags(EObjectFlags::RF_WasLoaded);
			DuplicateFragment->ClearFlags(EObjectFlags::RF_LoadCompleted);
			OwningInventoryComponent->AddReplicatedSubObject(DuplicateFragment);
			ChangedFragments.Add(DuplicateFragment);
			return DuplicateFragment;
		}
	}
	return nullptr;
}

TArray<UXIUItemFragment*> FXIUFragments::GetAllFragments() const
{
	//TODO: this way im getting both the default and changed fragments, which is not desirable 
	TArray<UXIUItemFragment*> Result = Super::GetAllFragments(); 
	Result.Reserve(Result.Num() + ChangedFragments.Num());
	for (UXIUItemFragment* Fragment : ChangedFragments)
	{
		Result.Add(Fragment);
	}
	return Result;
}


void FXIUFragments::Set(TSubclassOf<UXIUItemFragment> FragmentClass, UXIUItemFragment* NewFragment)
{
	checkf(FragmentClass, TEXT("To set a fragment, the class must be specified"))
	
	if (NewFragment)
	{
		// if fragment is default, then remove from changed fragments
		const UXIUItemFragment* DefaultFragment = FindDefaultFragmentByClass(FragmentClass);
		if (DefaultFragment && DefaultFragment->Matches(NewFragment))
		{
			Remove(FragmentClass);
			return;
		}

		// if the fragment is already in the changed fragments list, then do nothing
		if (ChangedFragments.Contains(NewFragment)) return;
	}

	// if it is not in the changed list, duplicate it and add it
	DuplicateAndAdd(NewFragment);
}

void FXIUFragments::Remove(TSubclassOf<UXIUItemFragment> FragmentClass)
{
	if (UClass* TargetClass = FragmentClass.Get())
	{
		for (auto FragmentIt = ChangedFragments.CreateIterator(); FragmentIt; ++FragmentIt)
		{
			UXIUItemFragment* Fragment = *FragmentIt;
			if (Fragment && Fragment->IsA(TargetClass))
			{
				OwningInventoryComponent->RemoveReplicatedSubObject(Fragment);
				FragmentIt.RemoveCurrent();
			}
		}
	}
}

void FXIUFragments::RemoveAll()
{
	for (auto FragmentIt = ChangedFragments.CreateIterator(); FragmentIt; ++FragmentIt)
	{
		if (UXIUItemFragment* Fragment = *FragmentIt)
		{
			OwningInventoryComponent->RemoveReplicatedSubObject(Fragment);
			FragmentIt.RemoveCurrent();
		}
	}
}

UXIUItemFragment* FXIUFragments::GetOrDefault(TSubclassOf<UXIUItemFragment> FragmentClass)
{
	// try to get the matching changed fragment 
	if (UClass* TargetClass = FragmentClass.Get())
	{
		for (UXIUItemFragment* Fragment : ChangedFragments)
		{
			if (Fragment && Fragment->IsA(TargetClass))
			{
				return Fragment;
			}
		}
	}

	// duplicate the default fragment and add it
	return DuplicateAndAdd(FindDefaultFragmentByClass(FragmentClass));
}

UXIUItemFragment* FXIUFragments::FindFragmentByClass(const TSubclassOf<UXIUItemFragment> FragmentClass, bool bCheckDefaults) const
{
	UXIUItemFragment* FoundFragment = nullptr;

	if (UClass* TargetClass = FragmentClass.Get())
	{
		for (UXIUItemFragment* Fragment : ChangedFragments)
		{
			if (Fragment && Fragment->IsA(TargetClass))
			{
				FoundFragment = Fragment;
				break;
			}
		}
	}

	if (!FoundFragment && bCheckDefaults) FoundFragment = FindDefaultFragmentByClass(FragmentClass);

	return FoundFragment;
}



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

bool UXIUItemFragment::Matches(UXIUItemFragment* Fragment) const
{
	return true;
}
