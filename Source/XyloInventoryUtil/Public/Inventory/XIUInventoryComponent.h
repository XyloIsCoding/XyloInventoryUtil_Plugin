// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "XIUInventoryComponent.generated.h"



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

// private:
// 	UPROPERTY(EditDefaultsOnly)
// 	FXIUInventory Inventory;
};
