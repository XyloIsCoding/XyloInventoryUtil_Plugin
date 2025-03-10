// Copyright XyloIsCoding 2024

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "XIUInventoryInterface.generated.h"

class UXIUInventoryComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UXIUInventoryInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class XYLOINVENTORYUTIL_API IXIUInventoryInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Item")
	UXIUInventoryComponent* GetInventoryComponent() const;
	virtual UXIUInventoryComponent* GetInventoryComponent_Implementation() const = 0;
};
