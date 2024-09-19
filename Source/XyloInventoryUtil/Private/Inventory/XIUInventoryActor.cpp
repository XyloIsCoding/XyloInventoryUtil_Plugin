// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryActor.h"

#include "Inventory/XIUInventoryComponent.h"


AXIUInventoryActor::AXIUInventoryActor()
{
	PrimaryActorTick.bCanEverTick = false;
	InventoryComponent = CreateDefaultSubobject<UXIUInventoryComponent>(TEXT("InventoryComponent"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * AActor Interface
 */

void AXIUInventoryActor::BeginPlay()
{
	Super::BeginPlay();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * IXIUPickUpInterface Interface
 */

UXIUItem* AXIUInventoryActor::GetItem_Implementation()
{
	if (InventoryComponent)
	{
		return InventoryComponent->GetFirstItem();
	}
	return nullptr;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * InventoryActor 
 */

void AXIUInventoryActor::UpdateItemMesh()
{
	if (UXIUItem* ItemToDisplay = Execute_GetItem(this))
	{
		if (UStaticMesh* NewMesh = ItemToDisplay->GetItemMesh())
		{
			ItemMesh = NewMesh;
			return;
		}
	}
	ItemMesh = nullptr;
}


