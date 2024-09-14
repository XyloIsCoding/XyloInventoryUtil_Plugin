// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XIUItemFragment.h"
#include "UObject/Object.h"
#include "XIUItem.generated.h"


struct FXIUDefaultFragments;
class UXIUItemStack;
class UXIUItemFragment;

/**
 * 
 */ 
UCLASS(Blueprintable, BlueprintType)
class XYLOINVENTORYUTIL_API UXIUItem : public UDataAsset
{
	GENERATED_BODY()

private:
	friend UXIUItemStack;
	
public:
	UPROPERTY(EditAnywhere, Category = "Item")
	FString Name;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	FXIUDefaultFragments Fragments;

private:
	virtual void Use(AActor* User, UXIUItemStack* ItemStack) const;
	virtual void UsageTick(AActor* User, UXIUItemStack* ItemStack, float DeltaSeconds) const;
	virtual void FinishUsing(AActor* User, UXIUItemStack* ItemStack) const;
};
