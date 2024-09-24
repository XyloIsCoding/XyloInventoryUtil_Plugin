// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XIUItem.h"
#include "XIUPickUpInterface.h"
#include "GameFramework/Actor.h"
#include "XIUItemActor.generated.h"

UCLASS()
class XYLOINVENTORYUTIL_API AXIUItemActor : public AActor, public IXIUPickUpInterface
{
	GENERATED_BODY()
	
public:	
	AXIUItemActor();


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
	virtual UXIUItem* GetItem_Implementation() override;
	virtual bool TryPickUp_Implementation(UXIUInventoryComponent* OtherInventory) override;
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * ItemActor 
	 */

private:
	UPROPERTY(EditAnywhere)
	FXIUItemDefault DefaultItem;

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	UXIUItem* Item;
public:
	UFUNCTION(BlueprintCallable)
	void SetItemWithDefault(FXIUItemDefault NewItemDefault);
	/** Duplicate the item and set the duplicate as this actor's item (does not touch the input item) */
	UFUNCTION(BlueprintCallable)
	void SetItem(UXIUItem* NewItem);
protected:
	virtual void ItemSet();
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "ItemSet"))
	void BP_ItemSet();
	

};