// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "XIUItemDefinition.generated.h"

class UXIUItemFragment;
class UXIUItem;
/**
 * 
 */
UCLASS()
class XYLOINVENTORYUTIL_API UXIUItemDefinition : public UDataAsset
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "Item")
	FString ItemName;
public:
	FString GetItemName() const { return ItemName; }
	
private:
	UPROPERTY(EditAnywhere, Category = "Item")
	TSubclassOf<UXIUItem> ItemClass;
	UPROPERTY()
	UXIUItem* Item;
public:
	const UXIUItem* GetItem();

public:
	UPROPERTY(EditAnywhere, Category = "Item")
	TArray<TObjectPtr<UXIUItemFragment>> DefaultFragments;

	/** @return Default Fragment of the desired class. You should NEVER modify fragments got in this way */
	UFUNCTION(BlueprintCallable, Category = "Fragments")
	UXIUItemFragment* GetDefaultFragment(const TSubclassOf<UXIUItemFragment> FragmentClass);
};
