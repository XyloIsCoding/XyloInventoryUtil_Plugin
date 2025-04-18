// Copyright XyloIsCoding 2024


#include "Inventory/XIUInventoryUtilLibrary.h"
#include "Inventory/XIUInventoryComponent.h"
#include "Inventory/Item/XIUItemDefinition.h"


UXIUItem* UXIUInventoryUtilLibrary::MakeItemFromDefault(UObject* Outer, FXIUItemDefault ItemDefault)
{
	checkf(ItemDefault.ItemDefinition && ItemDefault.ItemDefinition->ItemClass, TEXT("Cannot make item of unset class"))
	if (ItemDefault.Count <= 0) return nullptr; 

	UXIUItem* Item = NewObject<UXIUItem>(Outer, ItemDefault.ItemDefinition->ItemClass);
	Item->InitializeItem(ItemDefault);
	return Item;
}

UXIUItem* UXIUInventoryUtilLibrary::DuplicateItem(UObject* Outer, UXIUItem* Item)
{
	checkf(Item, TEXT("Cannot duplicate null item"));
	return Item->Duplicate(Outer);
}

const UXIUItemFragment* UXIUInventoryUtilLibrary::FindItemDefinitionFragment(const TSubclassOf<UXIUItemDefinition> ItemDef, const TSubclassOf<UXIUItemFragment> FragmentClass)
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<UXIUItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
	}
	return nullptr;
}
