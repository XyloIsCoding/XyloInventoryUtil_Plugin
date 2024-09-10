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

public:
	UPROPERTY(EditAnywhere, Category = "Item")
	FString Name;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	FXIUDefaultFragments Fragments;
};
