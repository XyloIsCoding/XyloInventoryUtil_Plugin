#pragma once

#include "XIUInventoryChangeMessage.generated.h"


class UXIUItem;

USTRUCT(BlueprintType)
struct FXIUInventorySlotChangeMessage
{
	GENERATED_BODY()

	//@TODO: Tag based names+owning actors for inventories instead of directly exposing the component?
	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	TObjectPtr<UActorComponent> InventoryOwner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 Index = 0;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	bool bItemChanged = false;
	
	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TObjectPtr<UXIUItem> Item = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 NewCount = 0;

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 Delta = 0;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TObjectPtr<UXIUItem> OldItem = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TSubclassOf<UXIUItem> Filter = nullptr;

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	bool bLocked = false;
};

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FXIUInventoryChangedSignature, const FXIUInventorySlotChangeMessage&, Change);
UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FXIUInventoryManualInitializationSignature);