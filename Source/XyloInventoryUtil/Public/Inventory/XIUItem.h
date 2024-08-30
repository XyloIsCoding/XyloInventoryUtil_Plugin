// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "XIUItem.generated.h"


class UXIUItemStack;

// Represents a fragment of an item definition
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class XYLOINVENTORYUTIL_API UXIUItemFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(UXIUItemStack* Instance) const {}
};

/**
 * 
 */
UCLASS(Blueprintable, Const, Abstract)
class XYLOINVENTORYUTIL_API UXIUItem : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Item")
	TArray<TObjectPtr<UXIUItemFragment>> Fragments;

public:
	UXIUItemStack* CreateItemStack();
};
