// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUInventoryComponent.h"

#include "Engine/ActorChannel.h"
#include "Inventory/XIUItemStack.h"
#include "Net/UnrealNetwork.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * FXIUInventorySlot Interface
 */


FString FXIUInventorySlot::GetDebugString() const
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
 * FXIUInventory Interface
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
	for (const FXIUInventorySlot& Entry : Entries)
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


	FXIUInventorySlot& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Stack = NewObject<UXIUItemStack>(OwnerComponent->GetOwner());  //@TODO: Using the actor instead of component as the outer due to UE-127172
	NewEntry.Stack->SetItem(ItemClass.GetDefaultObject());
	for (UXIUItemFragment* Fragment : GetDefault<UXIUItem>(ItemClass)->Fragments)
	{
		if (Fragment != nullptr)
		{
			NewEntry.Stack->AddFragment(Fragment);
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
		FXIUInventorySlot& Entry = *EntryIt;
		if (Entry.Stack == Stack)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

void FXIUInventoryList::BroadcastChangeMessage(FXIUInventorySlot& Entry, int32 OldCount, int32 NewCount)
{
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


UXIUInventoryComponent::UXIUInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Inventory(this)
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	//Inventory = FXIUInventoryList::FXIUInventory(3);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UObject Interface
 */

bool UXIUInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FXIUInventorySlot& Entry : Inventory.Entries)
	{
		UXIUItemStack* Instance = Entry.Stack;

		if (Instance && IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

void UXIUInventoryComponent::ReadyForReplication()
{
	Super::ReadyForReplication();
	
	// Register existing UXIUItemStack
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FXIUInventorySlot& Entry : Inventory.Entries)
		{
			UXIUItemStack* Instance = Entry.Stack;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
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

	DOREPLIFETIME(ThisClass, Inventory);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Inventory
 */

void UXIUInventoryComponent::AddDefaultItems()
{
	ServerAddDefaultItems();
}

void UXIUInventoryComponent::ServerAddDefaultItems_Implementation()
{
	if (GetOwner() && GetOwner()->HasAuthority())
    	{
    		for (TSubclassOf<UXIUItem> Item : DefaultItems)
    		{
    			Inventory.AddEntry(Item, 2);
    		}
    	}
}

void UXIUInventoryComponent::PrintItems()
{
	for (UXIUItemStack* Stack : Inventory.GetAllItems())
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Orange, FString::Printf(TEXT("Item: %s  ;  Count %i"), *Stack->GetItem()->Name, Stack->GetCount()));
	}
}






bool UXIUInventoryComponent::CanAddItemDefinition(TSubclassOf<UXIUItem> ItemDef, int32 StackCount)
{
	//@TODO: Add support for stack limit / uniqueness checks / etc...
	return true;
}

UXIUItemStack* UXIUInventoryComponent::AddItemDefinition(TSubclassOf<UXIUItem> ItemDef, int32 StackCount)
{
	UXIUItemStack* Result = nullptr;
	if (ItemDef != nullptr)
	{
		Result = Inventory.AddEntry(ItemDef, StackCount);
		
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Result)
		{
			AddReplicatedSubObject(Result);
		}
	}
	return Result;
}

void UXIUInventoryComponent::AddItemInstance(UXIUItemStack* ItemInstance)
{
	Inventory.AddEntry(ItemInstance);
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
	{
		AddReplicatedSubObject(ItemInstance);
	}
}

void UXIUInventoryComponent::RemoveItemInstance(UXIUItemStack* ItemInstance)
{
	Inventory.RemoveEntry(ItemInstance);

	if (ItemInstance && IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}
}

TArray<UXIUItemStack*> UXIUInventoryComponent::GetAllItems() const
{
	return Inventory.GetAllItems();
}

UXIUItemStack* UXIUInventoryComponent::FindFirstItemStackByDefinition(TSubclassOf<UXIUItem> ItemDef) const
{
	for (const FXIUInventorySlot& Entry : Inventory.Entries)
	{
		UXIUItemStack* Instance = Entry.Stack;

		if (IsValid(Instance))
		{
			if (Instance->GetItem()->GetClass() == ItemDef)
			{
				return Instance;
			}
		}
	}

	return nullptr;
}

int32 UXIUInventoryComponent::GetTotalItemCountByDefinition(TSubclassOf<UXIUItem> ItemDef) const
{
	int32 TotalCount = 0;
	for (const FXIUInventorySlot& Entry : Inventory.Entries)
	{
		UXIUItemStack* Instance = Entry.Stack;

		if (IsValid(Instance))
		{
			if (Instance->GetItem()->GetClass() == ItemDef)
			{
				++TotalCount;
			}
		}
	}

	return TotalCount;
}

bool UXIUInventoryComponent::ConsumeItemsByDefinition(TSubclassOf<UXIUItem> ItemDef, int32 NumToConsume)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority())
	{
		return false;
	}

	//@TODO: N squared right now as there's no acceleration structure
	int32 TotalConsumed = 0;
	while (TotalConsumed < NumToConsume)
	{
		if (UXIUItemStack* Instance = UXIUInventoryComponent::FindFirstItemStackByDefinition(ItemDef))
		{
			Inventory.RemoveEntry(Instance);
			++TotalConsumed;
		}
		else
		{
			return false;
		}
	}

	return TotalConsumed == NumToConsume;
}