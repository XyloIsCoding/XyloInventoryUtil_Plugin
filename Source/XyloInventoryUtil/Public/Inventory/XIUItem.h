// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "XIUItem.generated.h"


class UXIUItem;

USTRUCT(BlueprintType)
struct FXIUItemDefault
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UXIUItem> ItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Count;
};

/**
 * 
 */ 
UCLASS(Blueprintable, BlueprintType)
class XYLOINVENTORYUTIL_API UXIUItem : public UObject
{
	GENERATED_BODY()

public:
	UXIUItem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
private:
	UPROPERTY(EditAnywhere, Category = "Item")
	FString ItemName;
	UPROPERTY(EditAnywhere, Replicated, Category = "Item")
	int Count;
	UPROPERTY(EditAnywhere, Replicated, Category = "Item")
	int MaxCount;

private:
	UPROPERTY(EditAnywhere, Category = "Appearance")
	UStaticMesh* ItemMesh;
public:
	UFUNCTION(BlueprintCallable, Category = "Appearance")
	UStaticMesh* GetItemMesh() const { return ItemMesh; }

public:
	FString GetItemName() const;
	
	/** @return item count */
	UFUNCTION(BlueprintCallable)
	int GetCount() const;
	/** Set the count of the item */
	UFUNCTION(BlueprintCallable)
	void SetCount(int NewCount);
	/** @return count added */
	UFUNCTION(BlueprintCallable)
	int ModifyCount(const int AddCount);

	bool IsEmpty() const;
	virtual bool CanStack(UXIUItem* Item);
	virtual UXIUItem* Duplicate(UObject* Outer);
	
};
