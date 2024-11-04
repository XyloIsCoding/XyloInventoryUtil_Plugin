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
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	UXIUItem* GetItem();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	bool TryPickUp(UXIUInventoryComponent* OtherInventory);
};
