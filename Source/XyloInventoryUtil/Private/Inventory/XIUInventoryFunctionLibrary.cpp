// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryFunctionLibrary.h"
#include "Inventory/XIUInventoryComponent.h"
#include "Inventory/XIUItemDefinition.h"


UXIUItem* UXIUInventoryFunctionLibrary::MakeItemFromDefault(UObject* Outer, FXIUItemDefault ItemDefault)
{
	checkf(ItemDefault.ItemDefinition && ItemDefault.ItemDefinition->ItemClass, TEXT("Cannot make item of unset class"))
	
	UXIUItem* Item = NewObject<UXIUItem>(Outer, ItemDefault.ItemDefinition->ItemClass);
	Item->SetItemDefinition(ItemDefault.ItemDefinition);
	Item->SetCount(ItemDefault.Count);
	return Item;
}

UXIUItem* UXIUInventoryFunctionLibrary::DuplicateItem(UObject* Outer, UXIUItem* Item)
{
	checkf(Item, TEXT("Cannot duplicate null item"));
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
