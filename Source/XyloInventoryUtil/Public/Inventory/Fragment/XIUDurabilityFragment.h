// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/XIUItemFragment.h"
#include "XIUDurabilityFragment.generated.h"

/**
 * 
 */
UCLASS()
class XYLOINVENTORYUTIL_API UXIUDurabilityFragment : public UXIUItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float MaxDurability;

	UPROPERTY(EditAnywhere)
	float Durability;

public:
	virtual bool Matches(UXIUItemFragment* Fragment) const override;
};
