// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Fragment/XIUCountFragment.h"

#include "Net/UnrealNetwork.h"

void UXIUCountFragment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, MaxCount);
	DOREPLIFETIME(ThisClass, Count);
}
