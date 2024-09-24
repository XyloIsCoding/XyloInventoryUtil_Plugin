// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItem.h"

#include "Net/UnrealNetwork.h"


UXIUItem::UXIUItem(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	Count = 0;
	MaxCount = 1;
	ItemMesh = nullptr;
}

void UXIUItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Count);
	DOREPLIFETIME(ThisClass, MaxCount);
}

FString UXIUItem::GetItemName() const
{
	return ItemName;
}

int UXIUItem::GetCount() const
{
	return Count;
}

void UXIUItem::SetCount(int NewCount)
{
	Count = FMath::Clamp(NewCount, 0, MaxCount);
	UE_LOG(LogTemp, Warning, TEXT("Set Count %i (requested %i. MaxCount %i)"), Count, NewCount, MaxCount)
}

int UXIUItem::ModifyCount(const int AddCount)
{
	const int OldCount = Count;
	Count = FMath::Clamp(Count + AddCount, 0, MaxCount);
	return Count - OldCount;
}

bool UXIUItem::IsEmpty() const
{
	return Count == 0;
}

bool UXIUItem::IsFull() const
{
	return Count == MaxCount;
}

bool UXIUItem::CanStack(UXIUItem* Item)
{
	if (!Item->IsA(GetClass())) return false;
	return true;
}

UXIUItem* UXIUItem::Duplicate(UObject* Outer)
{
	UXIUItem* Item = NewObject<UXIUItem>(Outer, GetClass());
	Item->Count = Count;
	Item->MaxCount = MaxCount;
	return Item;
}
