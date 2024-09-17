// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "XIUItem.generated.h"


class UXIUItemStack;

/**
 * 
 */ 
UCLASS(Blueprintable, BlueprintType, DefaultToInstanced)
class XYLOINVENTORYUTIL_API UXIUItem : public UObject
{
	GENERATED_BODY()

private:
	friend UXIUItemStack;

protected:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	void Use(AActor* User, UXIUItemStack* ItemStack) const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	void UsageTick(AActor* User, UXIUItemStack* ItemStack, float DeltaSeconds) const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	void FinishUsing(AActor* User, UXIUItemStack* ItemStack) const;
};
