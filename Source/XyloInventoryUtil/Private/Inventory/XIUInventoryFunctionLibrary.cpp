// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryFunctionLibrary.h"

#include "Inventory/XIUItem.h"
#include "Inventory/XIUItemStack.h"
#include "Inventory/Fragment/XIUCountFragment.h"

UXIUItemStack* UXIUInventoryFunctionLibrary::MakeItemStackFromItem(UObject* Outer, TSubclassOf<UXIUItem> ItemClass)
{
	UXIUItemStack* ItemStack = NewObject<UXIUItemStack>(Outer);
	const UXIUItem* Item = GetDefault<UXIUItem>(ItemClass);
	ItemStack->SetItem(Item);

	for (UXIUItemFragment* Fragment : Item->Fragments)
	{
		if (Fragment)
		{
			UXIUItemFragment* NewFragment = DuplicateObject<UXIUItemFragment>(Fragment, Outer);
			ItemStack->AddFragment(NewFragment);
		}
	}

	//if (UXIUCountFragment* CountFrag = Cast<UXIUCountFragment>(Item->Fragments[0]))
	//{
	//	ItemStack->TestFragment = DuplicateObject<UXIUCountFragment>(CountFrag, Outer);
	//}

	ItemStack->TestFragment = NewObject<UXIUCountFragment>(Outer);
	ItemStack->TestFragment->MaxCount = 10;
	
	return ItemStack;
}
