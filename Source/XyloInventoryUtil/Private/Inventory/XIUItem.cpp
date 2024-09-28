// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItem.h"
#include "Net/UnrealNetwork.h"


UXIUItem::UXIUItem(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	MaxCount = 1;
}

AActor* UXIUItem::GetOwningActor() const
{
	return GetTypedOuter<AActor>();
}

void UXIUItem::DestroyObject()
{
	if (IsValid(this))
	{
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

	DOREPLIFETIME(ThisClass, Count);
	DOREPLIFETIME(ThisClass, MaxCount);
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
	SetCount(0);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Item
 */

void UXIUItem::OnRep_Count(int OldCount)
{
	if (OldCount == -1) return;
	ItemCountChangedDelegate.Broadcast(FXIUItemCountChangeMessage(this, Count, OldCount));
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
	const int OldCount = Count;
	Count = FMath::Clamp(NewCount, 0, MaxCount);
	ItemCountChangedDelegate.Broadcast(FXIUItemCountChangeMessage(this, Count, OldCount));

	//UE_LOG(LogTemp, Warning, TEXT("Set Count %i (requested %i. MaxCount %i)"), Count, NewCount, MaxCount)
}

int UXIUItem::ModifyCount(const int AddCount)
{
	const int OldCount = Count;
	Count = FMath::Clamp(Count + AddCount, 0, MaxCount);
	ItemCountChangedDelegate.Broadcast(FXIUItemCountChangeMessage(this, Count, OldCount));
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

