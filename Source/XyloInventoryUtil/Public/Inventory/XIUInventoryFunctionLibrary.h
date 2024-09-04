// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XIUInventoryFunctionLibrary.generated.h"

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
	static UXIUItemStack* MakeItemStackFromItem(UObject* Outer, TSubclassOf<UXIUItem> ItemClass);
};
