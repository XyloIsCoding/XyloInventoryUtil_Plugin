// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItemStack.h"

UXIUItem* UXIUItemStack::GetItem() const
{
	return Item;
}

int UXIUItemStack::GetCount()
{
	return Count;
}

void UXIUItemStack::SetCount(int NewCount)
{
	Count = NewCount;
}

void UXIUItemStack::AddCount(int AddCount)
{
	Count += AddCount;
}
