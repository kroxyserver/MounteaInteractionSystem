﻿// All rights reserved Dominik Pavlicek 2022.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "Interfaces/ActorInteractableInterface.h"
#include "ActorInteractableComponentBase.generated.h"

enum class EInteractableLifecycle : uint8;

UCLASS(ClassGroup=(Interaction), Blueprintable, hideCategories=(Collision, AssetUserData, Cooking, ComponentTick, Activation), meta=(BlueprintSpawnableComponent, DisplayName = "Interactable Component"))
class ACTORINTERACTIONPLUGIN_API UActorInteractableComponentBase : public UWidgetComponent, public IActorInteractableInterface
{
	GENERATED_BODY()

public:

	UActorInteractableComponentBase();

protected:

	virtual void BeginPlay() override;

protected:

	UFUNCTION()
	virtual bool DoesAutoSetup() const override;
	UFUNCTION()
	virtual void ToggleAutoSetup(const bool NewValue) override;

	UFUNCTION()
	virtual bool ActivateInteractable(FString& ErrorMessage) override;
	UFUNCTION()
	virtual bool WakeUpInteractable(FString& ErrorMessage) override;
	UFUNCTION()
	virtual bool SnoozeInteractable(FString& ErrorMessage) override;
	UFUNCTION()
	virtual bool CompleteInteractable(FString& ErrorMessage) override;
	UFUNCTION() 
	virtual void DeactivateInteractable() override;

	UFUNCTION()
	virtual EInteractableStateV2 GetState() const override;
	UFUNCTION()
	virtual void SetState(const EInteractableStateV2 NewState) override;

	UFUNCTION()
	virtual TScriptInterface<IActorInteractorInterface> GetInteractor() const override;
	UFUNCTION()
	virtual void SetInteractor(const TScriptInterface<IActorInteractorInterface> NewInteractor) override;

	UFUNCTION()
	virtual int32 GetInteractableWeight() const override;
	UFUNCTION()
	virtual void SetInteractableWeight(const int32 NewWeight) override;

	UFUNCTION()
	virtual AActor* GetInteractableOwner() const override;
	UFUNCTION()
	virtual void SetInteractableOwner(AActor* NewOwner) override;

	UFUNCTION()
	virtual ECollisionChannel GetCollisionChannel() const override;
	UFUNCTION()
	virtual void SetCollisionChannel(const ECollisionChannel& NewChannel) override;

	UFUNCTION()
	virtual TArray<UPrimitiveComponent*> GetCollisionComponents() const override;
	UFUNCTION()
	virtual void AddCollisionComponent(UPrimitiveComponent* CollisionComp) override;
	UFUNCTION()
	virtual void AddCollisionComponents(const TArray<UPrimitiveComponent*> NewCollisionComponents) override;
	UFUNCTION()
	virtual void RemoveCollisionComponent(UPrimitiveComponent* CollisionComp) override;
	UFUNCTION()
	virtual void RemoveCollisionComponents(const TArray<UPrimitiveComponent*> RemoveCollisionComponents) override;

	UFUNCTION()
	virtual TArray<UMeshComponent*> GetHighlightableComponents() const override;
	UFUNCTION()
	virtual void AddHighlightableComponent(UMeshComponent* HighlightableComp) override;
	UFUNCTION()
	virtual void AddHighlightableComponents(const TArray<UMeshComponent*> AddHighlightableComponents) override;
	UFUNCTION()
	virtual void RemoveHighlightableComponent(UMeshComponent* HighlightableComp) override;
	UFUNCTION()
	virtual void RemoveHighlightableComponents(const TArray<UMeshComponent*> RemoveHighlightableComponents) override;
	


	UFUNCTION()
	virtual UMeshComponent* FindMeshByTag(const FName Tag) const override;
	UFUNCTION()
	virtual UPrimitiveComponent* FindPrimitiveByTag(const FName Tag) const override;

	UFUNCTION()
	virtual TArray<FName> GetCollisionOverrides() const override;
	UFUNCTION()
	virtual void AddCollisionOverride(const FName Tag) override;
	UFUNCTION()
	virtual void AddCollisionOverrides(const TArray<FName> Tags) override;
	UFUNCTION()
	virtual void RemoveCollisionOverride(const FName Tag) override;
	UFUNCTION()
	virtual void RemoveCollisionOverrides(const TArray<FName> Tags) override;

	UFUNCTION()
	virtual TArray<FName> GetHighlightableOverrides() const override;
	UFUNCTION()
	virtual void AddHighlightableOverride(const FName Tag) override;
	UFUNCTION()
	virtual void AddHighlightableOverrides(const TArray<FName> Tags) override;
	UFUNCTION()
	virtual void RemoveHighlightableOverride(const FName Tag) override;
	UFUNCTION()
	virtual void RemoveHighlightableOverrides(const TArray<FName> Tags) override;

#pragma region EventFunctions

#pragma region NativeFunctions
	
	/***/
	UFUNCTION(Category="Interaction")
	void OnInteractableBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/***/
	UFUNCTION(Category="Interaction")
	void OnInteractableStopOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/***/
	UFUNCTION(Category="Interaction")
	void OnInteractableTraced(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	/***/
	UFUNCTION(Category="Interaction")
	virtual void InteractorFound(const TScriptInterface<IActorInteractorInterface>& FoundInteractor) override;

	/***/
	UFUNCTION(Category="Interaction")
	virtual void InteractorLost(const TScriptInterface<IActorInteractorInterface>& LostInteractor) override;

	/***/
	UFUNCTION(Category="Interaction")
	virtual void InteractionCompleted(const float& TimeCompleted) override;

	/***/
	UFUNCTION(Category="Interaction")
	virtual void InteractionStarted(const float& TimeStarted) override;

	/***/
	UFUNCTION(Category="Interaction")
	virtual void InteractionStopped() override;

	/***/
	UFUNCTION(Category="Interaction")
	virtual void InteractionLifecycleCompleted() override;

	/***/
	UFUNCTION(Category="Interaction")
	virtual void InteractionCooldownCompleted() override;

#pragma endregion

#pragma region InteractionEvents
	
	/**
	 * 
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void OnInteractorFoundEvent(const TScriptInterface<IActorInteractorInterface>& FoundInteractor);

	/**
	 * 
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void OnInteractorLostEvent(const TScriptInterface<IActorInteractorInterface>& LostInteractor);
	
	/**
	 * Event bound to OnInteractorOverlapped.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void OnInteractableBeginOverlapEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/**
	 * Event bound to OnInteractorStopOverlap.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void OnInteractableStopOverlapEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/**
	 * Event bound to OnInteractorTraced.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void OnInteractableTracedEvent(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void OnInteractionCompletedEvent(const float& FinishTime);
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void OnInteractionStartedEvent(const float& StartTime);
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void OnInteractionStoppedEvent();
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void OnLifecycleCompletedEvent();
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void OnCooldownCompletedEvent();

#pragma endregion 

#pragma region AttributesEvents

	/***/
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void OnInteractableAutoSetupChangedEvent(const bool NewValue);

	/***/
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void OnInteractableWeightChangedEvent(const int32& NewWeight);

	/***/
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void OnInteractableStateChangedEvent(const EInteractableStateV2& NewState);

	/***/
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void OnInteractableOwnerChangedEvent(const AActor* NewOwner);

	/***/
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void OnInteractableCollisionChannelChangedEvent(const ECollisionChannel NewChannel);

#pragma endregion 

#pragma endregion 
	
	virtual void BindCollisionEvents(UPrimitiveComponent* PrimitiveComponent) const override;
	virtual void UnbindCollisionEvents(UPrimitiveComponent* PrimitiveComponent) const override;
	
	/**
	 * Development Only.
	 * Toggles debug On/Off.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Interaction", meta=(DevelopmentOnly))
	virtual void ToggleDebug() override
	{
		bToggleDebug = !bToggleDebug;
	}

	/**
	 * Helper function.
	 * Looks for Collision and Highlightable Overrides and binds them.
	 * Looks for Parent Component which will be set as Collision Mesh and Highlighted Mesh.
	 */
	UFUNCTION()
	void AutoSetup();

protected:

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category="Interaction")
	FInteractorFound OnInteractorFound;
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category="Interaction")
	FInteractorLost OnInteractorLost;
	
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FInteractorTraced OnInteractorTraced;
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FInteractorOverlapped OnInteractorOverlapped;
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FInteractorStopOverlap OnInteractorStopOverlap;
	
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category="Interaction")
	FInteractionCompleted OnInteractionCompleted;
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category="Interaction")
	FInteractionStarted OnInteractionStarted;
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category="Interaction")
	FInteractionStopped OnInteractionStopped;
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FLifecycleCompleted OnLifecycleCompleted;
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FCooldownCompleted OnCooldownCompleted;

	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FInteractableAutoSetupChanged OnInteractableAutoSetupChanged;
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FInteractableWeightChanged OnInteractableWeightChanged;
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FInteractableStateChanged OnInteractableStateChanged;
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FInteractableOwnerChanged OnInteractableOwnerChanged;
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FInteractableCollisionChannelChanged OnInteractableCollisionChannelChanged;
	
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FHighlightableComponentAdded OnHighlightableComponentAdded;
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FHighlightableComponentRemoved OnHighlightableComponentRemoved;
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FCollisionComponentAdded OnCollisionComponentAdded;
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FCollisionComponentRemoved OnCollisionComponentRemoved;
	
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FHighlightableOverrideAdded OnHighlightableOverrideAdded;
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FHighlightableOverrideRemoved OnHighlightableOverrideRemoved;
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FCollisionOverrideAdded OnCollisionOverrideAdded;
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FCollisionOverrideRemoved OnCollisionOverrideRemoved;

protected:

	UPROPERTY(EditAnywhere, Category="Interaction|Required")
	uint8 bToggleDebug : 1;

	/**
	 * If set to true, Interactable will automatically assigns owning Component in Hierarchy as Highlightable Meshes and Collision Shapes.
	 * This setup might be useful for simple Actors, might cause issues with more complex ones.
	 */
	UPROPERTY(EditAnywhere, Category="Interaction|Required")
	uint8 bInteractableAutoActivate : 1;
	
	UPROPERTY(EditAnywhere, Category="Interaction|Required")
	uint8 bInteractableAutoSetup : 1;

	UPROPERTY(EditAnywhere, Category="Interaction|Required", meta=(NoResetToDefault))
	TEnumAsByte<ECollisionChannel> CollisionChannel;

	/**
	 * Clamped in setter function to be at least 0 or higher.
	 */
	UPROPERTY(EditAnywhere, Category="Interaction|Required", meta=(UIMin=0, ClampMin=0))
	int32 InteractionWeight;
	

	UPROPERTY(EditAnywhere, Category="Interaction|Optional", meta=(NoResetToDefault))
	EInteractableStateV2 DefaultInteractableState;
	UPROPERTY(EditAnywhere, Category="Interaction|Optional", meta=(NoResetToDefault))
	TArray<FName> CollisionOverrides;
	UPROPERTY(EditAnywhere, Category="Interaction|Optional", meta=(NoResetToDefault))
	TArray<FName> HighlightableOverrides;

	/**
	 * 
	 */
	UPROPERTY(EditAnywhere, Category="Interaction|Required", meta=(NoResetToDefault))
	EInteractableLifecycle LifecycleMode;
	UPROPERTY(EditAnywhere, Category="Interaction|Required", meta=(NoResetToDefault, EditCondition = "LifecycleMode == EInteractableLifecycle::EIL_Cycled"))
	int32 LifecycleCount;
	UPROPERTY(EditAnywhere, Category="Interaction|Required", meta=(NoResetToDefault))
	float CooldownPeriod;
	
	
	UPROPERTY(VisibleAnywhere, Category="Interaction|Read Only", meta=(NoResetToDefault))
	EInteractableStateV2 InteractableState;
	
	UPROPERTY(VisibleAnywhere, Category="Interaction|Read Only", meta=(NoResetToDefault))
	TArray<UMeshComponent*> HighlightableComponents;

	UPROPERTY(VisibleAnywhere, Category="Interaction|Read Only", meta=(NoResetToDefault))
	TArray<UPrimitiveComponent*> CollisionComponents;

private:

	/**
	 * Owning Actor of this Interactable.
	 * By default its set to Owner.
	 * Can be overriden.
	 */
	UPROPERTY(VisibleAnywhere, Category="Interaction|Read Only", meta=(NoResetToDefault, DisplayThumbnail = false))
	AActor* InteractionOwner;
	
	UPROPERTY(VisibleAnywhere, Category="Interaction|Read Only", meta=(NoResetToDefault, DisplayThumbnail = false))
	TScriptInterface<IActorInteractorInterface> Interactor;
};