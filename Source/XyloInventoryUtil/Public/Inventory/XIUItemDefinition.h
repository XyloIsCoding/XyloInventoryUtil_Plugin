// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "XIUItemDefinition.generated.h"

class UXIUItem;


UCLASS(DefaultToInstanced, EditInlineNew, Abstract, Blueprintable)
class XYLOINVENTORYUTIL_API UXIUItemFragment : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void OnInstanceCreated(UXIUItem* Item) const;
};

/**
 * 
 */
UCLASS(BlueprintType)
class XYLOINVENTORYUTIL_API UXIUItemDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UXIUItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<UXIUItem> ItemClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	FString ItemName;

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	int MaxCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item", Instanced)
	TArray<TObjectPtr<UXIUItemFragment>> Fragments;

public:
	UFUNCTION(BlueprintCallable, Category="Item")
	const UXIUItemFragment* FindFragmentByClass(const TSubclassOf<UXIUItemFragment> FragmentClass) const;

	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass());
	}
};
