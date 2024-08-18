// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryComponent.h"

#include "Inventory/XIUItemStack.h"


UXIUInventoryComponent::UXIUInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	//Inventory = FXIUInventory::FXIUInventory(3);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UActorComponent Interface
 */

void UXIUInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UXIUInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UXIUInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Inventory
 */
