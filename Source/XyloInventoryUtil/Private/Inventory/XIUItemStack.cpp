// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItemStack.h"

#include "Net/UnrealNetwork.h"

UXIUItemStack::UXIUItemStack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UXIUItemStack::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Item)
	DOREPLIFETIME(ThisClass, Count)
}

UXIUItemFragment* UXIUItemStack::AddFragment(UXIUItemFragment* ItemFragment)
{
	Fragments.AddFragment(ItemFragment);
	ItemFragment->OnInstanceCreated(this);
	return ItemFragment;
}

void UXIUItemStack::SetItem(UXIUItem* NewItem)
{
	Item = NewItem;
}

UXIUItem* UXIUItemStack::GetItem() const
{
	return Item;
}

int UXIUItemStack::GetCount()
{
	return Count;
}

void UXIUItemStack::SetCount(int NewCount)
{
	Count = NewCount;
}

void UXIUItemStack::AddCount(int AddCount)
{
	Count += AddCount;
}

void UXIUItemStack::AddFragments()
{
}
