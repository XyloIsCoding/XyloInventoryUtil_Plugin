// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryFunctionLibrary.h"

#include "Inventory/XIUItem.h"
#include "Inventory/XIUItemStack.h"
#include "Inventory/Fragment/XIUCountFragment.h"

UXIUItemStack* UXIUInventoryFunctionLibrary::MakeItemStackFromItem(UObject* Outer, TObjectPtr<UXIUItem> Item)
{
	if (!Item) return nullptr;
	
	UXIUItemStack* ItemStack = NewObject<UXIUItemStack>(Outer);
	ItemStack->SetItem(Item);

	for (UXIUItemFragment* Fragment : Item->Fragments)
	{
		if (Fragment)
		{
			if (UXIUItemFragment* NewFragment = DuplicateObject<UXIUItemFragment>(Fragment, Outer))
			{
			    // clear this two flags to allow client to update the outer of the duplicated fragment
				NewFragment->ClearFlags(EObjectFlags::RF_WasLoaded);
				NewFragment->ClearFlags(EObjectFlags::RF_LoadCompleted);
				ItemStack->AddFragment(NewFragment);
				UE_LOG(LogTemp, Warning, TEXT("Fragment added to new stack"))
			}
		}
	}
	
	return ItemStack;
}
