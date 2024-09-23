// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItemActor.h"

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

	SetItemWithDefault(DefaultItem);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * ItemActor 
 */

void AXIUItemActor::SetItemWithDefault(FXIUItemDefault NewItemDefault)
{
	if (NewItemDefault.ItemClass)
	{
		Item = UXIUInventoryFunctionLibrary::MakeItemFromDefault(this, NewItemDefault);
	}
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
}

