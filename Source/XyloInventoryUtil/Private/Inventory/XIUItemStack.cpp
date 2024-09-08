// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItemStack.h"

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

UXIUItemFragment* UXIUItemStack::AddFragment(UXIUItemFragment* ItemFragment)
{
	checkf(ItemFragment, TEXT("Tried to add a null fragment"));
	Fragments.AddFragment(ItemFragment);
	ItemFragment->OnInstanceCreated(this);
	return ItemFragment;
}

void UXIUItemStack::SetItem(const UXIUItem* NewItem)
{
	Item = NewItem;
}

const UXIUItem* UXIUItemStack::GetItem() const
{
	return Item;
}

int UXIUItemStack::GetCount()
{
	if (const UXIUCountFragment* CountFragment = Cast<UXIUCountFragment>(FindFragmentByClass(UXIUCountFragment::StaticClass())))
	{
		return CountFragment->Count;
	}
	return -1;
}

void UXIUItemStack::SetCount(int NewCount)
{
	if (UXIUCountFragment* CountFragment = Cast<UXIUCountFragment>(FindFragmentByClass(UXIUCountFragment::StaticClass())))
	{
		CountFragment->Count = FMath::Clamp(NewCount, 0, CountFragment->MaxCount);
	}
}

int UXIUItemStack::AddCount(int AddCount)
{
	if (UXIUCountFragment* CountFragment = Cast<UXIUCountFragment>(FindFragmentByClass(UXIUCountFragment::StaticClass())))
	{
		int PrevCount = CountFragment->Count;
		CountFragment->Count = FMath::Clamp(CountFragment->Count + AddCount, 0, CountFragment->MaxCount);
		return CountFragment->Count - PrevCount;
	}
	return 0;
}

UXIUItemFragment* UXIUItemStack::FindFragmentByClass(TSubclassOf<UXIUItemFragment> FragmentClass)
{
	return Fragments.GetFragment(FragmentClass);
}

