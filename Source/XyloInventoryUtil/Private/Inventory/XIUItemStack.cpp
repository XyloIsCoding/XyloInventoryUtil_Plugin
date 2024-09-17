// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItemStack.h"

#include "Inventory/XIUInventoryComponent.h"
#include "Inventory/XIUItem.h"
#include "Inventory/XIUItemDefinition.h"
#include "Inventory/Fragment/XIUCountFragment.h"
#include "Net/UnrealNetwork.h"

UXIUItemStack::UXIUItemStack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	ItemDefinition = nullptr;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UObject Interface
 */

void UXIUItemStack::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemDefinition)
	DOREPLIFETIME(ThisClass, Fragments)
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * ItemStack
 */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Owning Inventory */

void UXIUItemStack::SetOwningInventoryComponent(UXIUInventoryComponent* InventoryComponent)
{
	UXIUInventoryComponent* OldOwningComponent = OwningInventoryComponent;
	OwningInventoryComponent = InventoryComponent;

	// unregister from old inventory component owner
	if (OldOwningComponent) OldOwningComponent->UnregisterReplicatedObject(this);

	if (InventoryComponent)
	{
		// rename if necessary and register to new inventory component owner
		if (OldOwningComponent) Rename(nullptr, OwningInventoryComponent->GetOwner());
		OwningInventoryComponent->RegisterReplicatedObject(this);
	}

	// change fragments owner
	for (UXIUItemFragment* Fragment : Fragments)
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

/*--------------------------------------------------------------------------------------------------------------------*/
	
	
/*--------------------------------------------------------------------------------------------------------------------*/
/* Item Definition */

void UXIUItemStack::SetItemDefinition(UXIUItemDefinition* NewItemDefinition)
{
	ItemDefinition = NewItemDefinition;
	Fragments.Empty();
}

const UXIUItemDefinition* UXIUItemStack::GetItemDefinition() const
{
	return ItemDefinition;
}

/*--------------------------------------------------------------------------------------------------------------------*/
	
	
/*--------------------------------------------------------------------------------------------------------------------*/
/* Fragments */

TArray<UXIUItemFragment*> UXIUItemStack::GetAllFragments(const bool bCheckDefaults) const
{
	TArray<UXIUItemFragment*> ItemFragments = Fragments;

	if (bCheckDefaults && ItemDefinition)
	{
		TArray<UXIUItemFragment*> DefaultFragments;
		for (UXIUItemFragment* DefaultFragment : ItemDefinition->DefaultFragments)
		{
			// if ItemFragment does NOT contain already a fragment with this class then add the default version
			if (!ItemFragments.ContainsByPredicate([DefaultFragment](const UXIUItemFragment* Fragment) { return DefaultFragment->IsA(Fragment->GetClass()); } ))
			{
				DefaultFragments.Add(DefaultFragment);	
			}
		}
		ItemFragments.Append(DefaultFragments);
	}
	
	return ItemFragments;
}

UXIUItemFragment* UXIUItemStack::GetFragment(const TSubclassOf<UXIUItemFragment> FragmentClass) const
{
	UXIUItemFragment* FoundFragment = nullptr;

	if (UClass* TargetClass = FragmentClass.Get())
	{
		// look in item stack's fragments
		for (UXIUItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(TargetClass))
			{
				FoundFragment = Fragment;
				break;
			}
		}

		// look in default fragments
		if (!FoundFragment && ItemDefinition)
		{
			FoundFragment = ItemDefinition->GetDefaultFragment(FragmentClass);
		}
	}

	return FoundFragment;
}

UXIUItemFragment* UXIUItemStack::GetOrDefaultFragment(TSubclassOf<UXIUItemFragment> FragmentClass)
{
	// try to get the matching changed fragment 
	if (UClass* TargetClass = FragmentClass.Get())
	{
		for (UXIUItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(TargetClass))
			{
				return Fragment;
			}
		}
	}

	// duplicate the default fragment and add it
	if (ItemDefinition)
	{
		return DuplicateAndAdd(ItemDefinition->GetDefaultFragment(FragmentClass));
	}
	
	return nullptr;
}

void UXIUItemStack::SetFragment(TSubclassOf<UXIUItemFragment> FragmentClass, UXIUItemFragment* Fragment, bool bDuplicateFragment)
{
	checkf(FragmentClass, TEXT("To set a fragment, the class must be specified"))
	
	if (Fragment)
	{
		// if fragment is default, then remove from changed fragments
		if (ItemDefinition)
		{
			const UXIUItemFragment* DefaultFragment = ItemDefinition->GetDefaultFragment(FragmentClass);
			if (DefaultFragment && DefaultFragment->Matches(Fragment))
			{
				RemoveFragment(FragmentClass);
				return;
			}
		}

		// if the fragment is already in the changed fragments list, then do nothing
		if (Fragments.Contains(Fragment)) return;

		// if it is not in the changed list
		if (bDuplicateFragment)
		{
			DuplicateAndAdd(Fragment);
		}
		else
		{
			if (OwningInventoryComponent)
			{
				Fragment->Rename(nullptr, OwningInventoryComponent->GetOwner());
				OwningInventoryComponent->RegisterReplicatedObject(Fragment);
			}
			Fragments.Add(Fragment);
		}
	}
}

void UXIUItemStack::RemoveFragment(TSubclassOf<UXIUItemFragment> FragmentClass)
{
	if (UClass* TargetClass = FragmentClass.Get())
	{
		for (auto FragmentIt = Fragments.CreateIterator(); FragmentIt; ++FragmentIt)
		{
			UXIUItemFragment* Fragment = *FragmentIt;
			if (Fragment && Fragment->IsA(TargetClass))
			{
				if (OwningInventoryComponent) OwningInventoryComponent->UnregisterReplicatedObject(Fragment);
				FragmentIt.RemoveCurrent();
			}
		}
	}
}

void UXIUItemStack::RemoveAllFragments()
{
	for (auto FragmentIt = Fragments.CreateIterator(); FragmentIt; ++FragmentIt)
	{
		if (UXIUItemFragment* Fragment = *FragmentIt)
		{
			if (OwningInventoryComponent) OwningInventoryComponent->UnregisterReplicatedObject(Fragment);
			FragmentIt.RemoveCurrent();
		}
	}
}

UXIUItemFragment* UXIUItemStack::DuplicateAndAdd(const UXIUItemFragment* InFragment)
{
	if (InFragment && OwningInventoryComponent)
	{
		if (UXIUItemFragment* DuplicateFragment = InFragment->Duplicate(OwningInventoryComponent->GetOwner()))
		{
			OwningInventoryComponent->RegisterReplicatedObject(DuplicateFragment);
			Fragments.Add(DuplicateFragment);
			return DuplicateFragment;
		}
	}
	return nullptr;
}

int UXIUItemStack::GetCount()
{
	if (const UXIUCountFragment* CountFragment = GetFragment<UXIUCountFragment>())
	{
		return CountFragment->Count;
	}
	return -1;
}

int UXIUItemStack::SetCount(int NewCount)
{
	if (UXIUCountFragment* CountFragment = GetOrDefaultFragment<UXIUCountFragment>())
	{
		CountFragment->Count = FMath::Clamp(NewCount, 0, CountFragment->MaxCount);
		return CountFragment->Count;
	}
	return -1;
}

int UXIUItemStack::ModifyCount(int AddCount)
{
	if (UXIUCountFragment* CountFragment = GetOrDefaultFragment<UXIUCountFragment>())
	{
		int PrevCount = CountFragment->Count;
		CountFragment->Count = FMath::Clamp(CountFragment->Count + AddCount, 0, CountFragment->MaxCount);
		return CountFragment->Count - PrevCount;
	}
	return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/

	
/*--------------------------------------------------------------------------------------------------------------------*/
/* Stack */

UXIUItemStack* UXIUItemStack::Split(int Count)
{
	const int NewStackCount = -ModifyCount(-Count); // we remove count, and we are getting a negative number representing the count removed
	if (NewStackCount > 0)
	{
		if (UXIUItemStack* NewItemStack = Duplicate(nullptr))
		{
			NewItemStack->SetCount(NewStackCount);
			return NewItemStack;
		}
	}
	return nullptr;
}

UXIUItemStack* UXIUItemStack::Duplicate(UXIUInventoryComponent* InventoryComponent)
{
	UXIUInventoryComponent* NewOwningInventoryComponent = InventoryComponent ? InventoryComponent : OwningInventoryComponent.Get();
	
	UXIUItemStack* NewItemStack = NewObject<UXIUItemStack>( NewOwningInventoryComponent->GetOwner());
	NewItemStack->SetItemDefinition(ItemDefinition);
	// duplicate and add all changed fragments from original item stack
	for (const UXIUItemFragment* Fragment : Fragments)
	{
		if (Fragment)
		{
			NewItemStack->SetFragment(Fragment->GetClass(), Fragment->Duplicate(NewOwningInventoryComponent->GetOwner()));
		}
	}
	// we set the owning inventory for last, since this updates all fragments replication stuff too
	NewItemStack->SetOwningInventoryComponent(NewOwningInventoryComponent);
	
	return NewItemStack;
}

bool UXIUItemStack::IsEmpty()
{
	return GetCount() == 0;
}

bool UXIUItemStack::Matches(UXIUItemStack* ItemStack)
{
	if (ItemDefinition != ItemStack->GetItemDefinition()) return false;
	if (GetCount() != ItemStack->GetCount()) return false;

	TArray<UXIUItemFragment*> ThisFragments = GetAllFragments();
	TArray<UXIUItemFragment*> OtherFragments = ItemStack->GetAllFragments();
	if (ThisFragments.Num() != OtherFragments.Num()) return false;

	// compare fragments
	for (UXIUItemFragment* Fragment : ThisFragments)
	{
		if (UXIUItemFragment* SimilarFragment = ItemStack->GetFragment(Fragment->GetClass()))
		{
			// if a fragment does not match its similar fragment, then stacks are different
			if (!Fragment->Matches(SimilarFragment)) return false;
		}
		// if there is no fragment of the same class, then stacks are different
		else return false;
	}
	return true;
}

void UXIUItemStack::Use(AActor* User)
{
	if (ItemDefinition && ItemDefinition->GetItem())
	{
		ItemDefinition->GetItem()->Use(User, this);
	}
}

void UXIUItemStack::UsageTick(AActor* User, float DeltaSeconds)
{
	if (ItemDefinition && ItemDefinition->GetItem())
	{
		ItemDefinition->GetItem()->UsageTick(User, this, DeltaSeconds);
	}
}

void UXIUItemStack::FinishUsing(AActor* User)
{
	if (ItemDefinition && ItemDefinition->GetItem())
	{
		ItemDefinition->GetItem()->FinishUsing(User, this);
	}
}

/*--------------------------------------------------------------------------------------------------------------------*/
