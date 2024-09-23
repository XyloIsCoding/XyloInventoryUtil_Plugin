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
 * IXIUInventoryInterface Interface
 */

void AXIUInventoryActor::OnInventoryInitialized()
{
	BP_OnInventoryInitialized();
}

void AXIUInventoryActor::OnInventoryChanged()
{
	BP_OnInventoryChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * InventoryActor 
 */




