// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryFunctionLibrary.h"

#include "Inventory/XIUInventoryComponent.h"
#include "Inventory/XIUItem.h"
#include "Inventory/XIUItemStack.h"
#include "Inventory/Fragment/XIUCountFragment.h"

UXIUItemStack* UXIUInventoryFunctionLibrary::MakeItemStackFromItem(UXIUInventoryComponent* InventoryComponent, TObjectPtr<UXIUItem> Item)
{
	if (!Item) return nullptr;
	
	UXIUItemStack* ItemStack = NewObject<UXIUItemStack>(InventoryComponent->GetOwner());
	ItemStack->SetItem(Item);
	ItemStack->SetOwningInventoryComponent(InventoryComponent);
	return ItemStack;
}

UXIUItemStack* UXIUInventoryFunctionLibrary::DuplicateItemStack(UXIUInventoryComponent* InventoryComponent, TObjectPtr<UXIUItemStack> ItemStack)
{
	checkf(ItemStack, TEXT("Cannot duplicate null stack"));
	return ItemStack->Duplicate(InventoryComponent);
}
