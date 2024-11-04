// Copyright XyloIsCoding 2024

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XIUInventoryFunctionLibrary.generated.h"

class UXIUItemFragment;
class UXIUItemDefinition;
class UXIUItem;
struct FXIUItemDefault;
class UXIUInventoryComponent;
/**
 * 
 */
UCLASS()
class XYLOINVENTORYUTIL_API UXIUInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	static UXIUItem* MakeItemFromDefault(UObject* Outer, FXIUItemDefault ItemDefault);
	/** @param Outer: if nullptr keeps the ItemStack owner
	 * @param Item: the item to duplicate
	 * @return a new item stack, copy of the first one */
	UFUNCTION(BlueprintCallable)
	static UXIUItem* DuplicateItem(UObject* Outer, UXIUItem* Item);

	UFUNCTION(BlueprintCallable, meta=(DeterminesOutputType=FragmentClass))
	static const UXIUItemFragment* FindItemDefinitionFragment(const TSubclassOf<UXIUItemDefinition> ItemDef, const TSubclassOf<UXIUItemFragment> FragmentClass);
};
