// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "XIUItem.generated.h"


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
	
	UPROPERTY(EditAnywhere, Category = "Item", Instanced)
	TArray<TObjectPtr<UXIUItemFragment>> Fragments;
};
