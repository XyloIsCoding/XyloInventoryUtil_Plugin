// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "XIUInventoryComponent.generated.h"


class UXIUItem;
class UXIUItemStack;

USTRUCT()
struct FXIUInventorySlot
{
	GENERATED_BODY()

private:
	UPROPERTY()
	UXIUItemStack* ItemStack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UXIUItem* FilterItem;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShouldFilter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDisabled;

public:
	UXIUItemStack* GetItemStack();
	void SetItemStack(UXIUItemStack* ItemStack);

	UXIUItemStack* GetFilterItem();
	void SetFilterItem(UXIUItem* FilterItem);

	bool GetShouldFilter();
	void SetShouldFilter(bool bNewShouldFilter);
	
	bool GetDisabled();
	void SetDisabled(bool bNewDisabled);

	bool IsFull();
	bool CanInsert(UXIUItemStack* ItemStack);
	bool TryInsert(UXIUItemStack* ItemStack);
};

USTRUCT()
struct FXIUInventory
{
	GENERATED_BODY()

	FXIUInventory(int SlotsCount);
	
private:
	UPROPERTY()
	TArray<FXIUInventorySlot*> Slots;

public:
	bool AddItemStack(UXIUItemStack* ItemStack);
	bool RemoveItemStack(UXIUItemStack* ItemStack, int Count);
	FXIUInventorySlot* GetSlot(int Index);
	FXIUInventorySlot* GetSlot(UXIUItemStack* ItemStack);
};



UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class XYLOINVENTORYUTIL_API UXIUInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UXIUInventoryComponent();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UActorComponent Interface
	 */

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * Inventory
	 */

private:
	UPROPERTY()
	FXIUInventory* Inventory;
};
