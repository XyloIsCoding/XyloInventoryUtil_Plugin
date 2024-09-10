// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItemStackActor.h"

#include "Inventory/XIUInventoryFunctionLibrary.h"
#include "Inventory/XIUItemStack.h"


AXIUItemStackActor::AXIUItemStackActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * AActor Interface
 */

void AXIUItemStackActor::BeginPlay()
{
	Super::BeginPlay();

	//ItemStack = UXIUInventoryFunctionLibrary::MakeItemStackFromItem(this, DefaultItem);
	//ItemStack->SetCount(DefaultItemCount);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * IXIUPickUpInterface Interface
 */

UXIUItemStack* AXIUItemStackActor::GetItemStack_Implementation()
{
	return IXIUPickUpInterface::GetItemStack_Implementation();
}

