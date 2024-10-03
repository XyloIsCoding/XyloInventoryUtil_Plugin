// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItemActor.h"

#include "Inventory/XIUInventoryComponent.h"
#include "Inventory/XIUInventoryFunctionLibrary.h"

AXIUItemActor::AXIUItemActor()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	Item = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * AActor Interface
 */
	
void AXIUItemActor::BeginPlay()
{
	Super::BeginPlay();

	if (!Item && DefaultItem.ItemDefinition) SetItemWithDefault(DefaultItem);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * IXIUPickUpInterface Interface
 */

UXIUItem* AXIUItemActor::GetItem_Implementation()
{
	if (Item && !Item->IsEmpty()) return Item;
	return nullptr;
}

bool AXIUItemActor::TryPickUp_Implementation(UXIUInventoryComponent* OtherInventory)
{
	if (!OtherInventory) return false;
	
	if (UXIUItem* GotItem = Execute_GetItem(this))
	{
		OtherInventory->AddItem(GotItem);

		// no item count left
		if (GotItem->IsEmpty()) Destroy();
		return true;
	}
	
	// no item
	Destroy();
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * ItemActor 
 */

/*--------------------------------------------------------------------------------------------------------------------*/
/* Item Setter */

void AXIUItemActor::SetItemWithDefault(FXIUItemDefault NewItemDefault)
{
	Item = UXIUInventoryFunctionLibrary::MakeItemFromDefault(this, NewItemDefault);
	ItemSet();
}

void AXIUItemActor::SetItem(UXIUItem* NewItem)
{
	Item = UXIUInventoryFunctionLibrary::DuplicateItem(this, NewItem);
	ItemSet();
}

void AXIUItemActor::ItemSet()
{
	BP_ItemSet();

	// no item
	if (!Execute_GetItem(this)) Destroy();
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool AXIUItemActor::TryPickUpInSlot(UXIUInventoryComponent* OtherInventory, const int SlotIndex)
{
	if (!OtherInventory) return false;
	
	if (UXIUItem* GotItem = Execute_GetItem(this))
	{
		if (OtherInventory->SetItemAtSlot(SlotIndex, GotItem))
		{
			// item got fully picked up
			GotItem->SetCount(0);
		}

		// no item count left
		if (GotItem->IsEmpty()) Destroy();
		return true;
	}
	
	// no item
	Destroy();
	return false;
}