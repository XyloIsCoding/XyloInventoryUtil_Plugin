// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/XIUItemFragment.h"
#include "XIUCountFragment.generated.h"

/**
 * 
 */
UCLASS()
class XYLOINVENTORYUTIL_API UXIUCountFragment : public UXIUItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	int MaxCount;
	
	UPROPERTY(EditAnywhere)
	int Count;
};
