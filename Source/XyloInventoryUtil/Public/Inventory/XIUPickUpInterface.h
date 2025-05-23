// Copyright XyloIsCoding 2024

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "XIUPickUpInterface.generated.h"

class UXIUInventoryComponent;
class UXIUItem;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UXIUPickUpInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class XYLOINVENTORYUTIL_API IXIUPickUpInterface
{
	GENERATED_BODY()

public:
	/** @remark This function should not modify InItem's Count */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	void SetItem(UXIUItem* InItem, int32 Count);
	/** @remark This function should not modify InItem's Count */
	virtual void SetItem_Implementation(UXIUItem* InItem, int32 Count) = 0;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	UXIUItem* GetItem();
	virtual UXIUItem* GetItem_Implementation() = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	bool TryPickUp(UXIUInventoryComponent* OtherInventory);
	virtual bool TryPickUp_Implementation(UXIUInventoryComponent* OtherInventory) = 0;
};
