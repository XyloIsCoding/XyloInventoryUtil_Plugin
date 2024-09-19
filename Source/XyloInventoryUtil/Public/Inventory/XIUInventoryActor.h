// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XIUInventoryInterface.h"
#include "XIUItem.h"
#include "XIUPickUpInterface.h"
#include "GameFramework/Actor.h"
#include "XIUInventoryActor.generated.h"

struct FXIUInventoryChangeMessage;
class UXIUInventoryComponent;
struct FXIUItemDefault;

UCLASS()
class XYLOINVENTORYUTIL_API AXIUInventoryActor : public AActor, public IXIUInventoryInterface, public IXIUPickUpInterface
{
	GENERATED_BODY()
	
public:	
	AXIUInventoryActor();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * AActor Interface
	 */
	
protected:
	virtual void BeginPlay() override;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * IXIUInventoryInterface Interface
	 */

public:
	virtual UXIUInventoryComponent* GetInventoryComponent_Implementation() override { return InventoryComponent; }
	
private:
	UPROPERTY(EditDefaultsOnly)
	UXIUInventoryComponent* InventoryComponent;
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * IXIUPickUpInterface Interface
	 */

public:
	virtual UXIUItem* GetItem_Implementation() override;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * InventoryActor 
	 */

private:
	UPROPERTY(EditAnywhere)
	UStaticMesh* ItemMesh;

private:
	void UpdateItemMesh();
	
};
