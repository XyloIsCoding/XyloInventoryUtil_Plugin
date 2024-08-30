// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "XIUItemStack.generated.h"

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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	UPROPERTY(EditAnywhere, Category = "Item")
	UXIUItem* Item;
	UPROPERTY(EditAnywhere, Category = "Item")
	int Count;

public:
	void SetItem(UXIUItem* NewItem);
	UXIUItem* GetItem() const;
	int GetCount();
	void SetCount(int NewCount);
	void AddCount(int AddCount);

private:
	void AddFragments();
};
