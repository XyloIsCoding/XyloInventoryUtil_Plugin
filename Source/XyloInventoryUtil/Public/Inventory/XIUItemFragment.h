// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "XIUItemFragment.generated.h"


class UXIUInventoryComponent;
class UXIUItemFragment;
class UXIUItemStack;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * FXIUDefaultFragments
 */

/** Contains an array of fragments */
USTRUCT(BlueprintType)
struct FXIUDefaultFragments
{
	GENERATED_BODY()
	
public:
	virtual ~FXIUDefaultFragments() {}
	
protected:
	UPROPERTY(EditAnywhere, Category = "Item", Instanced)
	TArray<const UXIUItemFragment*> DefaultFragments;

public:
	virtual TArray<const UXIUItemFragment*> GetAllFragments() const;
	const UXIUItemFragment* FindDefaultFragmentByClass(const TSubclassOf<UXIUItemFragment> FragmentClass) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * FXIUFragments
 */

/** Contains an array of fragments */
USTRUCT(BlueprintType)
struct FXIUFragments : public FXIUDefaultFragments
{
	GENERATED_BODY()

public:
	FXIUFragments()
	{}
	
	FXIUFragments(const FXIUDefaultFragments& InDefaultFragments)
	{
		DefaultFragments = InDefaultFragments.GetAllFragments();
	}
	
private:
	UPROPERTY(NotReplicated)
	TObjectPtr<UXIUInventoryComponent> OwningInventoryComponent;
public:
	void SetOwningInventoryComponent(TObjectPtr<UXIUInventoryComponent> InventoryComponent);
	
protected:
	UPROPERTY(EditAnywhere, Category = "Item")
	TArray<TObjectPtr<UXIUItemFragment>> ChangedFragments;

private:
	UXIUItemFragment* DuplicateAndAdd(const UXIUItemFragment* NewFragment = nullptr);
	
public:
	virtual TArray<const UXIUItemFragment*> GetAllFragments() const override;
	void Set(TSubclassOf<UXIUItemFragment> FragmentClass, UXIUItemFragment* NewFragment);
	void Remove(TSubclassOf<UXIUItemFragment> FragmentClass);
	void RemoveAll();
	
	template<class T>
	T* GetOrDefault()
	{
		static_assert(TPointerIsConvertibleFromTo<T, const UXIUItemFragment>::Value, "'T' template parameter to GetOrDefault must be derived from UXIUItemFragment");

		return (T*)GetOrDefault(T::StaticClass());
	}
	UXIUItemFragment* GetOrDefault(TSubclassOf<UXIUItemFragment> FragmentClass);
	
	template<class T>
	const T* FindFragmentByClass() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, const UXIUItemFragment>::Value, "'T' template parameter to FindFragmentByClass must be derived from UXIUItemFragment");

		return (T*)FindFragmentByClass(T::StaticClass());
	}
	const UXIUItemFragment* FindFragmentByClass(const TSubclassOf<UXIUItemFragment> FragmentClass, bool bCheckDefaults = true) const;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXIUItemFragment
 */


/*
 * Represents a fragment of an item definition
 */
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class XYLOINVENTORYUTIL_API UXIUItemFragment : public UObject
{
	GENERATED_BODY()

public:
	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return bReplicated; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UObject interface
	
public:
	virtual void OnInstanceCreated(UXIUItemStack* Instance) const {}

	/** Defines whether two fragments are the same. Should be overridden by child classes */
	UFUNCTION(BlueprintCallable, Category = "Fragment")
	virtual bool Matches(UXIUItemFragment* Fragment) const;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Replication")
    bool bReplicated = true;
};
