// Copyright XyloIsCoding 2024

#pragma once

#include "CoreMinimal.h"
#include "XIUItem.h"
#include "GameFramework/Actor.h"
#include "Inventory/XIUPickUpInterface.h"
#include "XIUItemActor.generated.h"

UCLASS()
class XYLOINVENTORYUTIL_API AXIUItemActor : public AActor, public IXIUPickUpInterface
{
	GENERATED_BODY()
	
public:	
	AXIUItemActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


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
	/** Duplicate the item and set the duplicate as this actor's item (does not touch the input item) */
	UFUNCTION(BlueprintCallable)
	void SetItem_Implementation(UXIUItem* NewItem) override;
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
	TObjectPtr<UXIUItem> Item;
public:
	UFUNCTION(BlueprintCallable)
	void SetItemWithDefault(FXIUItemDefault NewItemDefault);
protected:
	UFUNCTION()
	void OnRep_Item(UXIUItem* OldItem);
protected:
	virtual void ItemSet();
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "Item Set"))
	void BP_ItemSet();

/*--------------------------------------------------------------------------------------------------------------------*/

public:
	virtual bool TryPickUpInSlot(UXIUInventoryComponent* OtherInventory, const int SlotIndex);
	

};
