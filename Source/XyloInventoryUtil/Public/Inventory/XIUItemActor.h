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
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


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
public:
	const FXIUItemDefault& GetDefaultItem() const { return DefaultItem; }

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Item Setter */
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_Item, BlueprintReadOnly, meta = (AllowPrivateAccess))
	UXIUItem* Item;
public:
	UFUNCTION(BlueprintCallable)
	void SetItemWithDefault(FXIUItemDefault NewItemDefault);
	/** Duplicate the item and set the duplicate as this actor's item (does not touch the input item) */
	UFUNCTION(BlueprintCallable)
	void SetItem(UXIUItem* NewItem);
protected:
	UFUNCTION()
	void OnRep_Item(UXIUItem* OldItem);
protected:
	virtual void ItemSet();
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "ItemSet"))
	void BP_ItemSet();

/*--------------------------------------------------------------------------------------------------------------------*/

public:
	virtual bool TryPickUpInSlot(UXIUInventoryComponent* OtherInventory, const int SlotIndex);
	

};
