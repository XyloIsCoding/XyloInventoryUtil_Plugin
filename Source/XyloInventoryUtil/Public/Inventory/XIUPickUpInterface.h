// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "XIUPickUpInterface.generated.h"

class UXIUItemStack;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UXIUPickUpInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class XYLOINVENTORYUTIL_API IXIUPickUpInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Item")
	UXIUItemStack* GetItemStack();
};
