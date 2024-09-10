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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	UPROPERTY(EditAnywhere, Replicated)
	int MaxCount;
	
	UPROPERTY(EditAnywhere, Replicated)
	int Count;

public:
	virtual bool Matches(UXIUItemFragment* Fragment) const override;
};
