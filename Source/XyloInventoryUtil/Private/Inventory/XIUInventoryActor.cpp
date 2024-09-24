// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryActor.h"

#include "Inventory/XIUInventoryComponent.h"


AXIUInventoryActor::AXIUInventoryActor()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	
	InventoryComponent = CreateDefaultSubobject<UXIUInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->InventoryInitializedServerDelegate.AddDynamic(this, &AXIUInventoryActor::OnInventoryInitialized);
	InventoryComponent->InventoryChangedServerDelegate.AddDynamic(this, &AXIUInventoryActor::OnInventoryChanged);
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
	return InventoryComponent->GetFirstItem();
}

bool AXIUInventoryActor::TryPickUp_Implementation(UXIUInventoryComponent* OtherInventory)
{
	if (!OtherInventory) return false;
	
	if (UXIUItem* Item = Execute_GetItem(this))
	{
		OtherInventory->AddItem(Item);
		InventoryComponent->ManuallyChangedInventory();
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
}

void AXIUInventoryActor::OnInventoryChanged()
{
	BP_OnInventoryChanged();
	
	if (bInventoryInitialized)
	{
		if (Execute_GetItem(this) == nullptr) Destroy();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * InventoryActor 
 */




