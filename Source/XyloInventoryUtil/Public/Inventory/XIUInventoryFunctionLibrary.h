// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XIUInventoryFunctionLibrary.generated.h"

class UXIUInventoryComponent;
class UXIUItem;
class UXIUItemStack;
/**
 * 
 */
UCLASS()
class XYLOINVENTORYUTIL_API UXIUInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static UXIUItemStack* MakeItemStackFromItem(UXIUInventoryComponent* InventoryComponent, TObjectPtr<UXIUItem> Item);
	/** @param InventoryComponent: if nullptr keeps the ItemStack owner
	 * @param ItemStack: the item stack to duplicate
	 * @return a new item stack, copy of the first one */
	static UXIUItemStack* DuplicateItemStack(UXIUInventoryComponent* InventoryComponent, TObjectPtr<UXIUItemStack> ItemStack);
};
