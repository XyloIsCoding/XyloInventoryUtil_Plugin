// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XIUInventoryFunctionLibrary.generated.h"

class UXIUItem;
struct FXIUItemDefault;
class UXIUInventoryComponent;
/**
 * 
 */
UCLASS()
class XYLOINVENTORYUTIL_API UXIUInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static UXIUItem* MakeItemFromDefault(UXIUInventoryComponent* InventoryComponent, FXIUItemDefault ItemDefault);
	/** @param InventoryComponent: if nullptr keeps the ItemStack owner
	 * @param Item: the item to duplicate
	 * @return a new item stack, copy of the first one */
	static UXIUItem* DuplicateItem(UXIUInventoryComponent* InventoryComponent, UXIUItem* Item);
};
