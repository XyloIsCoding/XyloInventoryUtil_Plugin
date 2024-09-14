// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItemStack.h"

#include "Inventory/XIUInventoryComponent.h"
#include "Inventory/XIUItem.h"
#include "Inventory/Fragment/XIUCountFragment.h"
#include "Net/UnrealNetwork.h"

UXIUItemStack::UXIUItemStack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	Item = nullptr;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UObject Interface
 */

void UXIUItemStack::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Item)
	DOREPLIFETIME(ThisClass, Fragments)
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * ItemStack
 */

void UXIUItemStack::SetOwningInventoryComponent(UXIUInventoryComponent* InventoryComponent)
{
	UXIUInventoryComponent* OldOwningComponent = OwningInventoryComponent;
	OwningInventoryComponent = InventoryComponent;

	// unregister from old inventory component owner
	if (OldOwningComponent) OldOwningComponent->UnregisterReplicatedObject(this);

	if (InventoryComponent)
	{
		// rename if necessary and register to new inventory component owner
		if (OldOwningComponent) Rename(nullptr, OwningInventoryComponent->GetOwner());
		OwningInventoryComponent->RegisterReplicatedObject(this);

		// change fragments owning inventory component
		Fragments.SetOwningInventoryComponent(InventoryComponent);
	}
}

void UXIUItemStack::SetItem(const UXIUItem* NewItem)
{
	Item = NewItem;
	Fragments.RemoveAll();
	Fragments = FXIUFragments(Item->Fragments);
}

const UXIUItem* UXIUItemStack::GetItem() const
{
	return Item;
}

TArray<UXIUItemFragment*> UXIUItemStack::GetAllFragments() const
{
	return Fragments.GetAllFragments();
}

void UXIUItemStack::SetFragment(TSubclassOf<UXIUItemFragment> FragmentClass, UXIUItemFragment* Fragment)
{
	Fragments.Set(FragmentClass, Fragment);
}

int UXIUItemStack::GetCount()
{
	if (const UXIUCountFragment* CountFragment = Fragments.FindFragmentByClass<UXIUCountFragment>())
	{
		return CountFragment->Count;
	}
	return -1;
}

int UXIUItemStack::SetCount(int NewCount)
{
	if (UXIUCountFragment* CountFragment = Fragments.GetOrDefault<UXIUCountFragment>())
	{
		CountFragment->Count = FMath::Clamp(NewCount, 0, CountFragment->MaxCount);
		return CountFragment->Count;
	}
	return -1;
}

int UXIUItemStack::ModifyCount(int AddCount)
{
	if (UXIUCountFragment* CountFragment = Fragments.GetOrDefault<UXIUCountFragment>())
	{
		int PrevCount = CountFragment->Count;
		CountFragment->Count = FMath::Clamp(CountFragment->Count + AddCount, 0, CountFragment->MaxCount);
		return CountFragment->Count - PrevCount;
	}
	return 0;
}

void UXIUItemStack::Use(AActor* User)
{
	if (Item) Item->Use(User, this);
}

void UXIUItemStack::UsageTick(AActor* User, float DeltaSeconds)
{
	if (Item) Item->UsageTick(User, this, DeltaSeconds);
}

void UXIUItemStack::FinishUsing(AActor* User)
{
	if (Item) Item->FinishUsing(User, this);
}
