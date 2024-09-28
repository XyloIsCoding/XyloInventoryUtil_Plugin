// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItem.h"

#include "Inventory/XIUInventoryFunctionLibrary.h"
#include "Inventory/XIUItemDefinition.h"
#include "Net/UnrealNetwork.h"


UXIUItem::UXIUItem(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	ItemDefinition = nullptr;
	bItemInitialized = false;
}

AActor* UXIUItem::GetOwningActor() const
{
	return GetTypedOuter<AActor>();
}

void UXIUItem::DestroyObject()
{
	if (IsValid(this))
	{
		SetCount(0); // setting count to zero to signal that we are destroying the item
		MarkAsGarbage();
		OnDestroyed();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UObject Interface
 */

void UXIUItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (const UBlueprintGeneratedClass* BPCClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPCClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	DOREPLIFETIME(ThisClass, ItemDefinition);
	DOREPLIFETIME(ThisClass, Count);
}

UWorld* UXIUItem::GetWorld() const
{
	if (GetOuter() == nullptr)
	{
		return nullptr;
	}
		
	if (Cast<UPackage>(GetOuter()) != nullptr)
	{
		return Cast<UWorld>(GetOuter()->GetOuter());
	}
		
	return GetOwningActor()->GetWorld();
}

int32 UXIUItem::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !IsSupportedForNetworking())
	{
		return GEngine->GetGlobalFunctionCallspace(Function, this, Stack);
	}
	
	return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool UXIUItem::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
{
	AActor* Owner = GetOwningActor();
	if (UNetDriver* NetDriver = Owner->GetNetDriver())
	{
		NetDriver->ProcessRemoteFunction(Owner, Function, Parms, OutParms, Stack, this);
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * IInterface_ActorSubobject Interface
 */

void UXIUItem::OnCreatedFromReplication()
{
	
}

void UXIUItem::OnDestroyedFromReplication()
{
	const int OldCount = GetCount();
	SetCount(0); // setting count to zero locally to signal that we are destroying the item (might have not replicated yet)
	ItemCountChangedDelegate.Broadcast(FXIUItemCountChangeMessage(this, OldCount));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Item
 */

void UXIUItem::SetItemDefinition(UXIUItemDefinition* InItemDefinition)
{
	checkf(!bItemInitialized, TEXT("Cannot reassign an item definition"))

	ItemDefinition = InItemDefinition;
	bItemInitialized = true;
}

void UXIUItem::OnRep_Count(int OldCount)
{
	if (OldCount != -1) // avoid broadcast on first replication cause it happens on creation (UXIUInventoryFunctionLibrary::MakeItemFromDefault)
	{
		ItemCountChangedDelegate.Broadcast(FXIUItemCountChangeMessage(this, OldCount));
	}
}

FString UXIUItem::GetItemName() const
{
	return ItemDefinition->ItemName;
}

int UXIUItem::GetCount() const
{
	return Count;
}

void UXIUItem::SetCount(int NewCount)
{
	const int OldCount = Count;
	Count = FMath::Clamp(NewCount, 0, ItemDefinition->MaxCount);
	if (GetOwningActor()->HasAuthority())
	{
		ItemCountChangedDelegate.Broadcast(FXIUItemCountChangeMessage(this, OldCount));
	}

	//UE_LOG(LogTemp, Warning, TEXT("Set Count %i (requested %i. MaxCount %i)"), Count, NewCount, MaxCount)
}

int UXIUItem::ModifyCount(const int AddCount)
{
	const int OldCount = Count;
	Count = FMath::Clamp(Count + AddCount, 0, ItemDefinition->MaxCount);
	if (GetOwningActor()->HasAuthority())
	{
		ItemCountChangedDelegate.Broadcast(FXIUItemCountChangeMessage(this, OldCount));
	}
	return Count - OldCount;
}

bool UXIUItem::IsEmpty() const
{
	return Count == 0;
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
	UXIUItem* Item = UXIUInventoryFunctionLibrary::MakeItemFromDefault(Outer, FXIUItemDefault(ItemDefinition, Count));
	return Item;
}

