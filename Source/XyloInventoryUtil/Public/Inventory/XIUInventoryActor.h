// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XIUInventoryInterface.h"
#include "GameFramework/Actor.h"
#include "XIUInventoryActor.generated.h"

struct FXIUInventorySlotChangeMessage;
class UXIUInventoryComponent;
struct FXIUItemDefault;

UCLASS()
class XYLOINVENTORYUTIL_API AXIUInventoryActor : public AActor, public IXIUInventoryInterface
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	UXIUInventoryComponent* InventoryComponent;

protected:
	UFUNCTION()
	virtual void OnInventoryInitialized();
	UFUNCTION()
	virtual void OnInventoryChanged();

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "OnInventoryInitialized"))
	void BP_OnInventoryInitialized();
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "OnInventoryChanged"))
	void BP_OnInventoryChanged();

	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * InventoryActor 
	 */
	
};
