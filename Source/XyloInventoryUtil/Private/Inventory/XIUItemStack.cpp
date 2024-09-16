// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItemStack.h"

#include "Inventory/XIUInventoryComponent.h"
#include "Inventory/XIUInventoryFunctionLibrary.h"
#include "Inventory/XIUItem.h"
#include "Inventory/Fragment/XIUCountFragment.h"
#include "Net/UnrealNetwork.h"

UXIUItemStack::UXIUItemStack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	Item = nullptr;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UObject Interface
 */

void UXIUItemStack::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Item)
	DOREPLIFETIME(ThisClass, Fragments)
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * ItemStack
 */

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

		// change fragments owning inventory component
		Fragments.SetOwningInventoryComponent(InventoryComponent);
	}
}

void UXIUItemStack::SetItem(const UXIUItem* NewItem)
{
	Item = NewItem;
	Fragments.RemoveAll();
	Fragments = FXIUFragments(Item->Fragments);
}

const UXIUItem* UXIUItemStack::GetItem() const
{
	return Item;
}

TArray<UXIUItemFragment*> UXIUItemStack::GetAllFragments(const bool bCheckDefaults) const
{
	return Fragments.GetAllFragments(bCheckDefaults);
}

UXIUItemFragment* UXIUItemStack::GetFragment(TSubclassOf<UXIUItemFragment> FragmentClass)
{
	return Fragments.FindFragmentByClass(FragmentClass);
}

void UXIUItemStack::SetFragment(TSubclassOf<UXIUItemFragment> FragmentClass, UXIUItemFragment* Fragment)
{
	Fragments.Set(FragmentClass, Fragment);
}

int UXIUItemStack::GetCount()
{
	if (const UXIUCountFragment* CountFragment = Fragments.FindFragmentByClass<UXIUCountFragment>())
	{
		return CountFragment->Count;
	}
	return -1;
}

int UXIUItemStack::SetCount(int NewCount)
{
	if (UXIUCountFragment* CountFragment = Fragments.GetOrDefault<UXIUCountFragment>())
	{
		CountFragment->Count = FMath::Clamp(NewCount, 0, CountFragment->MaxCount);
		return CountFragment->Count;
	}
	return -1;
}

int UXIUItemStack::ModifyCount(int AddCount)
{
	if (UXIUCountFragment* CountFragment = Fragments.GetOrDefault<UXIUCountFragment>())
	{
		int PrevCount = CountFragment->Count;
		CountFragment->Count = FMath::Clamp(CountFragment->Count + AddCount, 0, CountFragment->MaxCount);
		return CountFragment->Count - PrevCount;
	}
	return 0;
}

UXIUItemStack* UXIUItemStack::Split(int Count)
{
	const int NewStackCount = ModifyCount(Count);
	if (UXIUItemStack* NewItemStack = Duplicate(nullptr))
	{
		NewItemStack->SetCount(NewStackCount);
		return NewItemStack;
	}
	return nullptr;
}

UXIUItemStack* UXIUItemStack::Duplicate(UXIUInventoryComponent* InventoryComponent)
{
	UXIUInventoryComponent* NewOwningInventoryComponent = InventoryComponent ? InventoryComponent : OwningInventoryComponent.Get();
	
	UXIUItemStack* NewItemStack = NewObject<UXIUItemStack>( NewOwningInventoryComponent->GetOwner());
	NewItemStack->SetItem(Item);
	// duplicate and add all changed fragments from original item stack
	for (UXIUItemFragment* Fragment : Fragments.GetAllFragments(false))
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
	if (Item != ItemStack->GetItem()) return false;
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
	if (Item) Item->Use(User, this);
}

void UXIUItemStack::UsageTick(AActor* User, float DeltaSeconds)
{
	if (Item) Item->UsageTick(User, this, DeltaSeconds);
}

void UXIUItemStack::FinishUsing(AActor* User)
{
	if (Item) Item->FinishUsing(User, this);
}
