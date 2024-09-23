// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryFunctionLibrary.h"
#include "Inventory/XIUInventoryComponent.h"



UXIUItem* UXIUInventoryFunctionLibrary::MakeItemFromDefault(UObject* Outer, FXIUItemDefault ItemDefault)
{
	if (!ItemDefault.ItemClass) return nullptr;
	
	UXIUItem* Item = NewObject<UXIUItem>(Outer, ItemDefault.ItemClass);
	Item->SetCount(ItemDefault.Count);
	return Item;
}

UXIUItem* UXIUInventoryFunctionLibrary::DuplicateItem(UObject* Outer, UXIUItem* Item)
{
	checkf(Item, TEXT("Cannot duplicate null stack"));
	return Item->Duplicate(Outer);
}
