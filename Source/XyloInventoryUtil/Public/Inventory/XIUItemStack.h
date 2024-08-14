// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "XIUItemStack.generated.h"

class UXIUItem;
/**
 * 
 */
UCLASS()
class XYLOINVENTORYUTIL_API UXIUItemStack : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	UXIUItem* Item;
	int Count;

public:
	UXIUItem* GetItem() const;
	int GetCount();
	void SetCount(int NewCount);
	void AddCount(int AddCount);
};
