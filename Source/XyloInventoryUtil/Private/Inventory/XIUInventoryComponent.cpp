// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryComponent.h"

#include "Inventory/XIUItemStack.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * FXIUInventoryEntry Interface
 */


FString FXIUInventoryEntry::GetDebugString() const
{
	TSubclassOf<UXIUItem> Item;
	if (Stack != nullptr)
	{
		Item = Stack->GetItem()->GetClass();
	}

	return FString::Printf(TEXT("%s (%d x %s)"), *GetNameSafe(Stack), Stack->GetCount(), *GetNameSafe(Item));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * FXIUInventoryList Interface
 */


void FXIUInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
}

void FXIUInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
}

void FXIUInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
}

TArray<UXIUItemStack*> FXIUInventoryList::GetAllItems() const
{
	TArray<UXIUItemStack*> Results;
	Results.Reserve(Entries.Num());
	for (const FXIUInventoryEntry& Entry : Entries)
	{
		if (Entry.Stack != nullptr) //@TODO: Would prefer to not deal with this here and hide it further?
		{
			Results.Add(Entry.Stack);
		}
	}
	return Results;
}

UXIUItemStack* FXIUInventoryList::AddEntry(TSubclassOf<UXIUItem> ItemClass, int32 StackCount)
{
	UXIUItemStack* Result = nullptr;

	check(ItemClass != nullptr);
	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());


	FXIUInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Stack = NewObject<UXIUItemStack>(OwnerComponent->GetOwner());  //@TODO: Using the actor instead of component as the outer due to UE-127172
	NewEntry.Stack->SetItem(ItemClass.GetDefaultObject());
	for (UXIUItemFragment* Fragment : GetDefault<UXIUItem>(ItemClass)->Fragments)
	{
		if (Fragment != nullptr)
		{
			Fragment->OnInstanceCreated(NewEntry.Stack);
		}
	}
	NewEntry.Stack->SetCount(StackCount);
	Result = NewEntry.Stack;

	//const ULyraInventoryItemDefinition* ItemCDO = GetDefault<ULyraInventoryItemDefinition>(ItemDef);
	MarkItemDirty(NewEntry);

	return Result;
}

void FXIUInventoryList::AddEntry(UXIUItemStack* Stack)
{
	unimplemented();
}

void FXIUInventoryList::RemoveEntry(UXIUItemStack* Stack)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FXIUInventoryEntry& Entry = *EntryIt;
		if (Entry.Stack == Stack)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

void FXIUInventoryList::BroadcastChangeMessage(FXIUInventoryEntry& Entry, int32 OldCount, int32 NewCount)
{
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
