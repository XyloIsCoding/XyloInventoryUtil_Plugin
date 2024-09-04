// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryFunctionLibrary.h"

#include "Inventory/XIUItem.h"
#include "Inventory/XIUItemStack.h"

UXIUItemStack* UXIUInventoryFunctionLibrary::MakeItemStackFromItem(UObject* Outer, TSubclassOf<UXIUItem> ItemClass)
{
	UXIUItemStack* ItemStack = NewObject<UXIUItemStack>(Outer);
	const UXIUItem* Item = GetDefault<UXIUItem>(ItemClass);
	ItemStack->SetItem(Item);

	for (UXIUItemFragment* Fragment : Item->Fragments)
	{
		if (Fragment != nullptr)
		{
			ItemStack->AddFragment(Fragment);
		}
	}
	
	return ItemStack;
}
