// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryFunctionLibrary.h"

#include "Inventory/XIUInventoryComponent.h"
#include "Inventory/XIUItemStack.h"

UXIUItemStack* UXIUInventoryFunctionLibrary::MakeItemStackFromItem(UXIUInventoryComponent* InventoryComponent, TObjectPtr<UXIUItemDefinition> ItemDefinition)
{
	if (!ItemDefinition) return nullptr;
	
	UXIUItemStack* ItemStack = NewObject<UXIUItemStack>(InventoryComponent->GetOwner());
	ItemStack->SetItemDefinition(ItemDefinition);
	ItemStack->SetOwningInventoryComponent(InventoryComponent);
	return ItemStack;
}

UXIUItemStack* UXIUInventoryFunctionLibrary::DuplicateItemStack(UXIUInventoryComponent* InventoryComponent, TObjectPtr<UXIUItemStack> ItemStack)
{
	checkf(ItemStack, TEXT("Cannot duplicate null stack"));
	return ItemStack->Duplicate(InventoryComponent);
}
