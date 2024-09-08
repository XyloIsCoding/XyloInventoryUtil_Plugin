// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XIUPickUpInterface.h"
#include "GameFramework/Actor.h"
#include "XIUItemStackActor.generated.h"

class UXIUItem;

UCLASS()
class XYLOINVENTORYUTIL_API AXIUItemStackActor : public AActor, public IXIUPickUpInterface
{
	GENERATED_BODY()
	
public:	
	AXIUItemStackActor();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * AActor Interface
	 */
	
protected:
	virtual void BeginPlay() override;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * IXIUPickUpInterface Interface
	 */

public:
	virtual UXIUItemStack* GetItemStack_Implementation() override;
	
private:
	UPROPERTY(EditAnywhere, Category = Item)
	TObjectPtr<UXIUItem> DefaultItem;
	UPROPERTY(EditAnywhere, Category = Item)
	int DefaultItemCount;
	
	UPROPERTY()
	UXIUItemStack* ItemStack;
	
};
