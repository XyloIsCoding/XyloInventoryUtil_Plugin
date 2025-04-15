// Copyright XyloIsCoding 2024


#include "Inventory/Item/XIUItemActor.h"

#include "Inventory/XIUInventoryComponent.h"
#include "Inventory/XIUInventoryUtilLibrary.h"
#include "Net/UnrealNetwork.h"

AXIUItemActor::AXIUItemActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	bReplicateUsingRegisteredSubObjectList = true;
	Item = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * AActor Interface
 */
	
void AXIUItemActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && !Item && DefaultItem.ItemDefinition)
	{
		SetItemWithDefault(DefaultItem);
	}
}

void AXIUItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Item);
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
	Item = UXIUInventoryUtilLibrary::MakeItemFromDefault(this, NewItemDefault);
	OnRep_Item(nullptr);
}

void AXIUItemActor::SetItem(UXIUItem* NewItem)
{
	UXIUItem* OldItem = Item;
	Item = UXIUInventoryUtilLibrary::DuplicateItem(this, NewItem);
	OnRep_Item(OldItem);
}

void AXIUItemActor::OnRep_Item(UXIUItem* OldItem)
{
	if (IsUsingRegisteredSubObjectList())
	{
		if (IsValid(OldItem)) RemoveReplicatedSubObject(OldItem);
		if (IsValid(Item)) AddReplicatedSubObject(Item);
	}
	ItemSet();
}

void AXIUItemActor::ItemSet()
{
	BP_ItemSet();

	if (HasAuthority())
	{
		// no item
		if (!Execute_GetItem(this)) Destroy();
	}
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