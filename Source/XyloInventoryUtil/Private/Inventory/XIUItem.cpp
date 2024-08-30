// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItem.h"

#include "Inventory/XIUItemStack.h"

UXIUItemStack* UXIUItem::CreateItemStack()
{
	UXIUItemStack* ItemStack = NewObject<UXIUItemStack>();
	ItemStack->SetItem(this);
	return ItemStack;
}
