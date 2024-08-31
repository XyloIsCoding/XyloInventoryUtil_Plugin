// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XIUItemFragment.h"
#include "UObject/Object.h"
#include "XIUItemStack.generated.h"

struct FXIUFragments;
class UXIUItemFragment;
class UXIUItem;
/**
 * 
 */
UCLASS(BlueprintType)
class XYLOINVENTORYUTIL_API UXIUItemStack : public UObject
{
	GENERATED_BODY()

public:
	UXIUItemStack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


public:
	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UObject interface
	
private:
	UPROPERTY(Replicated, EditAnywhere, Category = "Item")
	UXIUItem* Item;
	UPROPERTY(Replicated, EditAnywhere, Category = "Item")
	int Count;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	FXIUFragments Fragments;
public:
	UXIUItemFragment* AddFragment(UXIUItemFragment* ItemFragment);

public:
	void SetItem(UXIUItem* NewItem);
	UXIUItem* GetItem() const;
	int GetCount();
	void SetCount(int NewCount);
	void AddCount(int AddCount);

private:
	void AddFragments();
};
