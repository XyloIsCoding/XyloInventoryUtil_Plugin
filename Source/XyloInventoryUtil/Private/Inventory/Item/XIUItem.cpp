// Copyright XyloIsCoding 2024


#include "Inventory/Item/XIUItem.h"

#include "Inventory/XIUInventoryUtilLibrary.h"
#include "Inventory/Item/XIUItemDefinition.h"
#include "Net/UnrealNetwork.h"


UXIUItem::UXIUItem(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	ItemDefinition = nullptr;
	bItemInitialized = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UObject Interface
 */

void UXIUItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ItemInitializer, COND_InitialOnly);
	DOREPLIFETIME(ThisClass, Count);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXROUReplicatedObject Interface
 */

void UXIUItem::OnDestroyed()
{
	if (bIsActive)
	{
		DestroyActiveState();
	}
	
	Super::OnDestroyed();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Item
 */

FString UXIUItem::GetItemName() const
{
	return ItemDefinition->ItemName;
}

bool UXIUItem::IsEmpty() const
{
	return Count <= 0;
}

bool UXIUItem::IsFull() const
{
	return Count == ItemDefinition->MaxCount;
}

bool UXIUItem::CanStack(UXIUItem* Item)
{
	if (Item->ItemDefinition != ItemDefinition) return false;
	return true;
}

UXIUItem* UXIUItem::Duplicate(UObject* Outer)
{
	UXIUItem* Item = UXIUInventoryUtilLibrary::MakeItemFromDefault(Outer, FXIUItemDefault(ItemDefinition, Count));
	return Item;
}

bool UXIUItem::IsItemAvailable(UXIUItem* Item)
{
	return Item && Item->IsItemInitialized() && !Item->IsEmpty();
}

bool UXIUItem::IsItemInitialized(UXIUItem* Item)
{
	return Item && Item->IsItemInitialized();
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Initialization */

bool UXIUItem::IsItemInitialized() const
{
	return bItemInitialized;
}

void UXIUItem::InitializeItem(const FXIUItemDefault& InItemInitializer)
{
	ItemInitializer = InItemInitializer;
	OnRep_ItemInitializer();
}

void UXIUItem::OnRep_ItemInitializer()
{
	InitializingItem();
}

void UXIUItem::InitializingItem()
{
	bIsActive = true;
	
	SetItemDefinition(ItemInitializer.ItemDefinition);
	SetCount(ItemInitializer.Count);
	bItemInitialized = true;
	
	OnItemInitialized();
	ItemInitializedDelegate.Broadcast(this);
}

void UXIUItem::OnItemInitialized()
{
}

void UXIUItem::DestroyActiveState()
{
	SetCount(0); // setting count to zero to signal that we are destroying the item
	bIsActive = false;
}

/*--------------------------------------------------------------------------------------------------------------------*/
	
/*--------------------------------------------------------------------------------------------------------------------*/
/* ItemDefinition */

void UXIUItem::SetItemDefinition(UXIUItemDefinition* InItemDefinition)
{
	checkf(!bItemInitialized, TEXT("Cannot reassign an item definition"))
	checkf(InItemDefinition, TEXT("Item definition must be valid"))

	ItemDefinition = InItemDefinition;
	for (UXIUItemFragment* Fragment : ItemDefinition->Fragments)
	{
		if (Fragment != nullptr)
		{
			Fragment->OnInstanceCreated(this);
		}
	}
}

/*--------------------------------------------------------------------------------------------------------------------*/
	
/*--------------------------------------------------------------------------------------------------------------------*/
/* Count */

int UXIUItem::GetCount() const
{
	return Count;
}

void UXIUItem::SetCount(int32 NewCount)
{
	const int OldCount = Count;
	Count = FMath::Clamp(NewCount, 0, ItemDefinition->MaxCount);
	LastCount = OldCount; // TODO: Remove

	OnRep_Count(OldCount);
	//UE_LOG(LogTemp, Warning, TEXT("Set Count %i (requested %i. MaxCount %i)"), Count, NewCount, ItemDefinition->MaxCount)
}

int UXIUItem::ModifyCount(const int AddCount)
{
	const int OldCount = Count;
	SetCount(Count + AddCount);
	
	return Count - OldCount;
}

void UXIUItem::OnRep_Count(int32 OldCount)
{
	// checking OldCount != -1 allow to block execution if it is the first count replication
	if (bItemInitialized && OldCount != -1) 
	{
		if (Count != OldCount) ItemCountChangedDelegate.Broadcast(FXIUItemCountChangeMessage(this, OldCount));
	}
}

/*--------------------------------------------------------------------------------------------------------------------*/
	