// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryFunctionLibrary.h"
#include "Inventory/XIUInventoryComponent.h"
#include "Inventory/XIUItemDefinition.h"


UXIUItem* UXIUInventoryFunctionLibrary::MakeItemFromDefault(UObject* Outer, FXIUItemDefault ItemDefault)
{
	if (!ItemDefault.ItemDefinition || !ItemDefault.ItemDefinition->ItemClass) return nullptr;
	
	UXIUItem* Item = NewObject<UXIUItem>(Outer, ItemDefault.ItemDefinition->ItemClass);
	Item->SetItemDefinition(ItemDefault.ItemDefinition);
	Item->SetCount(ItemDefault.Count);
	return Item;
}

UXIUItem* UXIUInventoryFunctionLibrary::DuplicateItem(UObject* Outer, UXIUItem* Item)
{
	checkf(Item, TEXT("Cannot duplicate null stack"));
	return Item->Duplicate(Outer);
}

const UXIUItemFragment* UXIUInventoryFunctionLibrary::FindItemDefinitionFragment(const TSubclassOf<UXIUItemDefinition> ItemDef, const TSubclassOf<UXIUItemFragment> FragmentClass)
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<UXIUItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
	}
	return nullptr;
}
