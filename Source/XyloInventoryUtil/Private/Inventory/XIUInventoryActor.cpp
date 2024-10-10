// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryActor.h"

#include "Inventory/XIUInventoryComponent.h"


AXIUInventoryActor::AXIUInventoryActor()
{
	bReplicates = true;
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

	if (InventoryComponent)
	{
		InventoryComponent->InventoryInitializedDelegate.AddUniqueDynamic(this, &AXIUInventoryActor::OnInventoryInitialized);
		InventoryComponent->InventoryChangedDelegate.AddUniqueDynamic(this, &AXIUInventoryActor::OnInventoryChanged);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * IXIUPickUpInterface Interface
 */

UXIUItem* AXIUInventoryActor::GetItem_Implementation()
{
	return InventoryComponent->GetFirstItem();
}

bool AXIUInventoryActor::TryPickUp_Implementation(UXIUInventoryComponent* OtherInventory)
{
	if (!OtherInventory) return false;
	
	if (UXIUItem* GotItem = Execute_GetItem(this))
	{
		OtherInventory->AddItem(GotItem);
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * IXIUInventoryInterface Interface
 */

void AXIUInventoryActor::OnInventoryInitialized()
{
	bInventoryInitialized = true;
	BP_OnInventoryInitialized();

	if (bDestroyOnEmpty && Execute_GetItem(this) == nullptr) Destroy();
}

void AXIUInventoryActor::OnInventoryChanged(const FXIUInventorySlotChangeMessage& Change)
{
	BP_OnInventoryChanged();
	
	if (bInventoryInitialized)
	{
		if (bDestroyOnEmpty && Execute_GetItem(this) == nullptr) Destroy();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * InventoryActor 
 */




