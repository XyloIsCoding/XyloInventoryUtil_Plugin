// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/XIUItem.h"

#include "Inventory/XIUItemStack.h"

void UXIUItem::Use_Implementation(AActor* User, UXIUItemStack* ItemStack) const
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Use Item")));
}

void UXIUItem::UsageTick_Implementation(AActor* User, UXIUItemStack* ItemStack, float DeltaSeconds) const
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Still Using")));
}

void UXIUItem::FinishUsing_Implementation(AActor* User, UXIUItemStack* ItemStack) const
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Finished Using")));
}
