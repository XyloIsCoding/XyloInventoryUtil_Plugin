// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Item/XIUItemDefinition.h"
#include "XIUDropFragment.generated.h"

/**
 * 
 */
UCLASS()
class XYLOINVENTORYUTIL_API UXIUDropFragment : public UXIUItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop", meta = (MustImplement = "/Script/XyloInventoryUtil.XIUPickUpInterface"))
	TSubclassOf<AActor> ItemDropActor;
};
