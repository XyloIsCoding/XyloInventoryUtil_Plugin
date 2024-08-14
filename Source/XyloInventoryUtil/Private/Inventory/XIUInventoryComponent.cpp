// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryComponent.h"

#include "Inventory/XIUItemStack.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * ItemStack
 */

UXIUItemStack* FXIUInventorySlot::GetItemStack()
{
}

void FXIUInventorySlot::SetItemStack(UXIUItemStack* ItemStack)
{
}

UXIUItemStack* FXIUInventorySlot::GetFilterItem()
{
}

void FXIUInventorySlot::SetFilterItem(UXIUItem* FilterItem)
{
}

bool FXIUInventorySlot::GetShouldFilter()
{
}

void FXIUInventorySlot::SetShouldFilter(bool bNewShouldFilter)
{
}

bool FXIUInventorySlot::GetDisabled()
{
}

void FXIUInventorySlot::SetDisabled(bool bNewDisabled)
{
}

bool FXIUInventorySlot::IsFull()
{
}

bool FXIUInventorySlot::CanInsert(UXIUItemStack* ItemStack)
{
	if (!ItemStack) return false;;
	return !GetDisabled() && (!GetFilterItem() || (GetFilterItem()->GetItem() == ItemStack->GetItem() && !IsFull()));
}

bool FXIUInventorySlot::TryInsert(UXIUItemStack* ItemStack)
{
	if (CanInsert(ItemStack))
	{
		GetItemStack()->AddCount(ItemStack->GetCount()); // TODO: it has to return the count that did not fit into the stack (max count defined in Item)
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Inventory
 */

FXIUInventory::FXIUInventory(int SlotsCount)
{
	Slots = TArray<FXIUInventorySlot*>(nullptr, SlotsCount);
}

bool FXIUInventory::AddItemStack(UXIUItemStack* ItemStack)
{
	TArray<FXIUInventorySlot*> AvailableSlots = Slots.FilterByPredicate([ItemStack](FXIUInventorySlot* Slot)
	{
		return Slot->CanInsert(ItemStack);
	});
	
	for (FXIUInventorySlot* InventorySlot : AvailableSlots)
	{
		InventorySlot->TryInsert(ItemStack);
		if (ItemStack->GetCount() == 0) break;
	}
}

bool FXIUInventory::RemoveItemStack(UXIUItemStack* ItemStack, int Count)
{
	
}

FXIUInventorySlot* FXIUInventory::GetSlot(int Index)
{
}

FXIUInventorySlot* FXIUInventory::GetSlot(UXIUItemStack* ItemStack)
{
}











UXIUInventoryComponent::UXIUInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
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
