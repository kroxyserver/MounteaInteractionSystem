﻿// All rights reserved Dominik Pavlicek 2022.

#include "Components/ActorInteractableComponentBase.h"

#include "Helpers/ActorInteractionPluginLog.h"

#if WITH_EDITOR
#include "EditorHelper.h"
#endif


#include "Interfaces/ActorInteractionWidget.h"
#include "Widgets/ActorInteractableWidget.h"

#include "TimerManager.h"
#include "Components/BillboardComponent.h"
#include "Components/WidgetComponent.h"
#include "Helpers/ActorInteractionFunctionLibrary.h"
#include "Helpers/MounteaInteractionSystemBFL.h"
#include "Interfaces/ActorInteractorInterface.h"

#define LOCTEXT_NAMESPACE "InteractableComponentBase"

UActorInteractableComponentBase::UActorInteractableComponentBase() :
		DefaultInteractableState(EInteractableStateV2::EIS_Awake),
		InteractableState(EInteractableStateV2::EIS_Awake),
		LifecycleCount(-1),
		RemainingLifecycleCount(LifecycleCount),
		InteractionWeight(1),
		CachedInteractionWeight(InteractionWeight),
		bInteractionHighlight(true),
		StencilID(133),
		HighlightType(EHighlightType::EHT_OverlayMaterial),
		LifecycleMode(EInteractableLifecycle::EIL_Cycled),
		InteractionPeriod(1.5f),
		CooldownPeriod(3.0f),
		ComparisonMethod(ETimingComparison::ECM_None),
		TimeToStart(0.001f),
		InteractableName(LOCTEXT("InteractableComponentBase", "Base")),
		SetupType(ESetupType::EST_Quick),
		bCanPersist(false),
		DebugSettings(false)
{
	bAutoActivate = true;
	
	SetIsReplicatedByDefault(true);
	SetActiveFlag(true);

	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	DebugSettings.DebugMode = false;
	DebugSettings.EditorDebugMode = false;

	Space = EWidgetSpace::Screen;
	DrawSize = FIntPoint(64, 64);
	bDrawAtDesiredSize = false;
	
	UActorComponent::SetActive(true);
	SetHiddenInGame(true);

#if WITH_EDITORONLY_DATA
	bVisualizeComponent = true;
#endif
}

void UActorInteractableComponentBase::BeginPlay()
{
	Super::BeginPlay();

	// Interaction Events
	OnInteractableSelected.											AddUniqueDynamic(this, &UActorInteractableComponentBase::OnInteractableSelectedEvent);
	OnInteractorFound.													AddUniqueDynamic(this, &UActorInteractableComponentBase::InteractorFound);
	OnInteractorLost.													AddUniqueDynamic(this, &UActorInteractableComponentBase::InteractorLost);

	OnInteractorOverlapped.										AddUniqueDynamic(this, &UActorInteractableComponentBase::OnInteractableBeginOverlapEvent);
	OnInteractorStopOverlap.										AddUniqueDynamic(this, &UActorInteractableComponentBase::OnInteractableStopOverlapEvent);
	OnInteractorTraced.												AddUniqueDynamic(this, &UActorInteractableComponentBase::OnInteractableTraced);

	OnInteractionCompleted.											AddUniqueDynamic(this, &UActorInteractableComponentBase::InteractionCompleted);
	OnInteractionCycleCompleted.								AddUniqueDynamic(this, &UActorInteractableComponentBase::InteractionCycleCompleted);
	OnInteractionStarted.												AddUniqueDynamic(this, &UActorInteractableComponentBase::InteractionStarted);
	OnInteractionStopped.											AddUniqueDynamic(this, &UActorInteractableComponentBase::InteractionStopped);
	OnInteractionCanceled.											AddUniqueDynamic(this, &UActorInteractableComponentBase::InteractionCanceled);
	OnLifecycleCompleted.											AddUniqueDynamic(this, &UActorInteractableComponentBase::InteractionLifecycleCompleted);
	OnCooldownCompleted.											AddUniqueDynamic(this, &UActorInteractableComponentBase::InteractionCooldownCompleted);
	
	// Attributes Events
	OnInteractableDependencyChanged.						AddUniqueDynamic(this, &UActorInteractableComponentBase::OnInteractableDependencyChangedEvent);
	OnInteractableAutoSetupChanged.						AddUniqueDynamic(this, &UActorInteractableComponentBase::OnInteractableAutoSetupChangedEvent);
	OnInteractableWeightChanged.								AddUniqueDynamic(this, &UActorInteractableComponentBase::OnInteractableWeightChangedEvent);
	OnInteractableStateChanged.									AddUniqueDynamic(this, &UActorInteractableComponentBase::OnInteractableStateChangedEvent);
	OnInteractableOwnerChanged.								AddUniqueDynamic(this, &UActorInteractableComponentBase::OnInteractableOwnerChangedEvent);
	OnInteractableCollisionChannelChanged.				AddUniqueDynamic(this, &UActorInteractableComponentBase::OnInteractableCollisionChannelChangedEvent);
	OnLifecycleModeChanged.										AddUniqueDynamic(this, &UActorInteractableComponentBase::OnLifecycleModeChangedEvent);
	OnLifecycleCountChanged.									AddUniqueDynamic(this, &UActorInteractableComponentBase::OnLifecycleCountChangedEvent);
	OnCooldownPeriodChanged.									AddUniqueDynamic(this, &UActorInteractableComponentBase::OnCooldownPeriodChangedEvent);
	OnInteractorChanged.												AddUniqueDynamic(this, &UActorInteractableComponentBase::OnInteractorChangedEvent);

	// Ignored Classes Events
	OnIgnoredInteractorClassAdded.							AddUniqueDynamic(this, &UActorInteractableComponentBase::OnIgnoredClassAdded);
	OnIgnoredInteractorClassRemoved.						AddUniqueDynamic(this, &UActorInteractableComponentBase::OnIgnoredClassRemoved);

	// Highlight Events
	OnHighlightableComponentAdded.							AddUniqueDynamic(this, &UActorInteractableComponentBase::OnHighlightableComponentAddedEvent);
	OnHighlightableComponentRemoved.						AddUniqueDynamic(this, &UActorInteractableComponentBase::OnHighlightableComponentRemovedEvent);
	
	// Collision Events
	OnCollisionComponentAdded.									AddUniqueDynamic(this, &UActorInteractableComponentBase::OnCollisionComponentAddedEvent);
	OnCollisionComponentRemoved.							AddUniqueDynamic(this, &UActorInteractableComponentBase::OnCollisionComponentRemovedEvent);

	// Widget
	OnWidgetUpdated.													AddUniqueDynamic(this, &UActorInteractableComponentBase::OnWidgetUpdatedEvent);
	
	// Highlight
	OnHighlightTypeChanged.										AddUniqueDynamic(this, &UActorInteractableComponentBase::OnHighlightTypeChangedEvent);
	OnHighlightMaterialChanged.									AddUniqueDynamic(this, &UActorInteractableComponentBase::OnHighlightMaterialChangedEvent);

	// Dependency
	InteractableDependencyStarted.							AddUniqueDynamic(this, &UActorInteractableComponentBase::InteractableDependencyStartedCallback);
	InteractableDependencyStopped.							AddUniqueDynamic(this, &UActorInteractableComponentBase::InteractableDependencyStoppedCallback);

	// Activation
	OnComponentActivated.											AddUniqueDynamic(this, &UActorInteractableComponentBase::InteractableComponentActivated);
	
	RemainingLifecycleCount = LifecycleCount;
	
	Execute_SetState(this, DefaultInteractableState);

	if (bAutoActivate)
	{
		AutoSetup();
	}

#if WITH_EDITOR
	
	DrawDebug();

#endif
}

void UActorInteractableComponentBase::InitWidget()
{
	Super::InitWidget();

	UpdateInteractionWidget();
}

void UActorInteractableComponentBase::OnRegister()
{
	
#if WITH_EDITORONLY_DATA
	if (bVisualizeComponent && SpriteComponent == nullptr && GetOwner() && !GetWorld()->IsGameWorld() )
	{
		SpriteComponent = NewObject<UBillboardComponent>(GetOwner(), NAME_None, RF_Transactional | RF_Transient | RF_TextExportTransient);

		SpriteComponent->Sprite = LoadObject<UTexture2D>(nullptr, TEXT("/ActorInteractionPlugin/Textures/Editor/T_MounteaLogo"));
		SpriteComponent->SetRelativeScale3D_Direct(FVector(1.f));
		SpriteComponent->Mobility = EComponentMobility::Movable;
		SpriteComponent->AlwaysLoadOnClient = false;
		SpriteComponent->SetIsVisualizationComponent(true);
		SpriteComponent->SpriteInfo.Category = TEXT("Misc");
		SpriteComponent->SpriteInfo.DisplayName = NSLOCTEXT( "SpriteCategory", "Misc", "Misc" );
		SpriteComponent->CreationMethod = CreationMethod;
		SpriteComponent->bIsScreenSizeScaled = true;
		SpriteComponent->bUseInEditorScaling = true;

		SpriteComponent->SetupAttachment(this);
		SpriteComponent->RegisterComponent();
	}
#endif
	
	Super::OnRegister();
}

bool UActorInteractableComponentBase::DoesHaveInteractor_Implementation() const
{
	return Interactor.GetObject() != nullptr;
}

#pragma region InteractionImplementations

bool UActorInteractableComponentBase::DoesAutoSetup_Implementation() const
{ return SetupType != ESetupType::EST_None; }

void UActorInteractableComponentBase::ToggleAutoSetup_Implementation(const ESetupType& NewValue)
{
	SetupType = NewValue;
}

bool UActorInteractableComponentBase::ActivateInteractable_Implementation(FString& ErrorMessage)
{
	const EInteractableStateV2 CachedState = InteractableState;

	Execute_SetState(this, EInteractableStateV2::EIS_Active);

	switch (CachedState)
	{
		case EInteractableStateV2::EIS_Active:
			ErrorMessage.Append(TEXT("Interactable Component is already Active"));
			break;
		case EInteractableStateV2::EIS_Awake:
			ErrorMessage.Append(TEXT("Interactable Component has been Activated"));
			return true;
		case EInteractableStateV2::EIS_Asleep:
		case EInteractableStateV2::EIS_Suppressed:
		case EInteractableStateV2::EIS_Cooldown:
		case EInteractableStateV2::EIS_Completed:
		case EInteractableStateV2::EIS_Disabled:
			ErrorMessage.Append(TEXT("Interactable Component cannot be Activated"));
			break;
		case EInteractableStateV2::Default: 
		default:
			ErrorMessage.Append(TEXT("Interactable Component cannot proces activation request, invalid state"));
			break;
	}
	
	return false;
}

bool UActorInteractableComponentBase::WakeUpInteractable_Implementation(FString& ErrorMessage)
{
	const EInteractableStateV2 CachedState = InteractableState;

	Execute_SetState(this, EInteractableStateV2::EIS_Awake);

	switch (CachedState)
	{
		case EInteractableStateV2::EIS_Awake:
			ErrorMessage.Append(TEXT("Interactable Component is already Awake"));
			break;
		case EInteractableStateV2::EIS_Active:
		case EInteractableStateV2::EIS_Asleep:
		case EInteractableStateV2::EIS_Suppressed:
		case EInteractableStateV2::EIS_Cooldown:
		case EInteractableStateV2::EIS_Disabled:
			ErrorMessage.Append(TEXT("Interactable Component has been Awaken"));
			return true;
		case EInteractableStateV2::EIS_Completed:
			ErrorMessage.Append(TEXT("Interactable Component cannot be Awaken"));
			break;
		case EInteractableStateV2::Default: 
		default:
			ErrorMessage.Append(TEXT("Interactable Component cannot proces activation request, invalid state"));
			break;
	}
	
	return false;
}

bool UActorInteractableComponentBase::CompleteInteractable_Implementation(FString& ErrorMessage)
{
	const EInteractableStateV2 CachedState = InteractableState;

	Execute_SetState(this, EInteractableStateV2::EIS_Completed);

	switch (CachedState)
	{
		case EInteractableStateV2::EIS_Active:
			ErrorMessage.Append(TEXT("Interactable Component is Completed"));
			return true;
		case EInteractableStateV2::EIS_Asleep:
		case EInteractableStateV2::EIS_Suppressed:
		case EInteractableStateV2::EIS_Awake:
		case EInteractableStateV2::EIS_Disabled:
		case EInteractableStateV2::EIS_Cooldown:
			ErrorMessage.Append(TEXT("Interactable Component cannot be Completed"));
			break;
		case EInteractableStateV2::EIS_Completed:
			ErrorMessage.Append(TEXT("Interactable Component is already Completed"));
			break;
		case EInteractableStateV2::Default: 
		default:
			ErrorMessage.Append(TEXT("Interactable Component cannot proces activation request, invalid state"));
			break;
	}
	
	return false;
}

void UActorInteractableComponentBase::DeactivateInteractable_Implementation()
{
	Execute_SetState(this, EInteractableStateV2::EIS_Disabled);
}

void UActorInteractableComponentBase::PauseInteraction_Implementation(const float ExpirationTime, const TScriptInterface<IActorInteractorInterface>& CausingInteractor)
{
	if (!GetWorld()) return;
	
	Execute_SetState(this, EInteractableStateV2::EIS_Paused);
	const bool bIsUnlimited = FMath::IsWithinInclusive(InteractionProgressExpiration, -1.f, 0.f) || FMath::IsNearlyZero(InteractionProgressExpiration, 0.001f);

	const float expirationTime = GetWorld()->GetTimeSeconds();
	if (bCanPersist)
	{
		GetWorld()->GetTimerManager().PauseTimer(Timer_Interaction);
		
		FTimerDelegate TimerDelegate_ProgressExpiration;
		
		TimerDelegate_ProgressExpiration.BindUFunction(this, "OnInteractionProgressExpired", expirationTime, CausingInteractor);

		const float ClampedExpiration = FMath::Max(InteractionProgressExpiration, 0.01f);
		
		GetWorld()->GetTimerManager().SetTimer(Timer_ProgressExpiration, TimerDelegate_ProgressExpiration, ClampedExpiration, false);
		GetWorld()->GetTimerManager().PauseTimer(Timer_Interaction);
	}
	else
	{
		OnInteractionProgressExpired(expirationTime, CausingInteractor);
	}
}

bool UActorInteractableComponentBase::CanInteract_Implementation() const
{
	if (!GetWorld()) return false;
	
	switch (InteractableState)
	{
		case EInteractableStateV2::EIS_Awake:
		case EInteractableStateV2::EIS_Active:
		case EInteractableStateV2::EIS_Paused:
			return Execute_GetInteractor(this).GetInterface() != nullptr;
		case EInteractableStateV2::EIS_Asleep:
		case EInteractableStateV2::EIS_Disabled:
		case EInteractableStateV2::EIS_Cooldown:
		case EInteractableStateV2::EIS_Completed:
		case EInteractableStateV2::EIS_Suppressed:
		case EInteractableStateV2::Default: 
		default: break;
	}
	
	return false;
}

bool UActorInteractableComponentBase::CanBeTriggered_Implementation() const
{
	if (!GetWorld()) return false;
	
	switch (InteractableState)
	{
		case EInteractableStateV2::EIS_Awake:
		case EInteractableStateV2::EIS_Active:
		case EInteractableStateV2::EIS_Paused:
			return true;
		case EInteractableStateV2::EIS_Asleep:
		case EInteractableStateV2::EIS_Disabled:
		case EInteractableStateV2::EIS_Cooldown:
		case EInteractableStateV2::EIS_Completed:
		case EInteractableStateV2::EIS_Suppressed:
		case EInteractableStateV2::Default: 
		default: break;
	}
	
	return false;
}

bool UActorInteractableComponentBase::IsInteracting_Implementation() const
{
	if (GetWorld())
	{
		return GetWorld()->GetTimerManager().IsTimerActive(Timer_Interaction);
	}

	LOG_ERROR(TEXT("[IsInteracting] Cannot find World!"))
	return false;
}

EInteractableStateV2 UActorInteractableComponentBase::GetDefaultState_Implementation() const
{ return DefaultInteractableState; }

void UActorInteractableComponentBase::SetDefaultState_Implementation(const EInteractableStateV2 NewState)
{
	if
	(
		DefaultInteractableState == EInteractableStateV2::EIS_Active ||
		DefaultInteractableState == EInteractableStateV2::EIS_Completed ||
		DefaultInteractableState == EInteractableStateV2::EIS_Cooldown
	)
	{
		LOG_ERROR(TEXT("[SetDefaultState] Tried to set invalid Default State!"))
		return;
	}
	DefaultInteractableState = NewState;
}

EInteractableStateV2 UActorInteractableComponentBase::GetState_Implementation() const
{ return InteractableState; }

void UActorInteractableComponentBase::CleanupComponent()
{
	Execute_StopHighlight(this);
	OnInteractableStateChanged.Broadcast(InteractableState);
	if (GetWorld()) GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	OnInteractorLost.Broadcast(Interactor);

	Execute_RemoveHighlightableComponents(this, HighlightableComponents);
	Execute_RemoveCollisionComponents(this, CollisionComponents);
}

void UActorInteractableComponentBase::SetState_Implementation(const EInteractableStateV2 NewState)
{
	switch (NewState)
	{
		case EInteractableStateV2::EIS_Active:
			switch (InteractableState)
			{
				case EInteractableStateV2::EIS_Paused:
				case EInteractableStateV2::EIS_Awake:
					InteractableState = NewState;
					OnInteractableStateChanged.Broadcast(InteractableState);
					break;
				case EInteractableStateV2::EIS_Active:
					break;
				case EInteractableStateV2::EIS_Asleep:
				case EInteractableStateV2::EIS_Suppressed:
				case EInteractableStateV2::EIS_Cooldown:
				case EInteractableStateV2::EIS_Completed:
				case EInteractableStateV2::EIS_Disabled:
				case EInteractableStateV2::Default:
				default: break;
			}
			break;
		case EInteractableStateV2::EIS_Awake:
			switch (InteractableState)
			{
				case EInteractableStateV2::EIS_Active:
				case EInteractableStateV2::EIS_Asleep:
				case EInteractableStateV2::EIS_Suppressed:
				case EInteractableStateV2::EIS_Cooldown:
				case EInteractableStateV2::EIS_Disabled:
				case EInteractableStateV2::EIS_Paused:
					{
						InteractableState = NewState;
						OnInteractableStateChanged.Broadcast(InteractableState);

						for (const auto& Itr : CollisionComponents)
						{
							Execute_BindCollisionShape(this, Itr);
						}
					}
					break;
				case EInteractableStateV2::EIS_Completed:
				case EInteractableStateV2::EIS_Awake:
				case EInteractableStateV2::Default: 
				default: break;
			}
			break;
		case EInteractableStateV2::EIS_Asleep:
			switch (InteractableState)
			{
				case EInteractableStateV2::EIS_Active:
				case EInteractableStateV2::EIS_Paused:
				case EInteractableStateV2::EIS_Awake:
				case EInteractableStateV2::EIS_Suppressed:
				case EInteractableStateV2::EIS_Cooldown:
				case EInteractableStateV2::EIS_Disabled:
					{
						InteractableState = NewState;

						// Replacing Cleanup
						Execute_StopHighlight(this);
						OnInteractableStateChanged.Broadcast(InteractableState);
						if (GetWorld()) GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
						OnInteractorLost.Broadcast(Interactor);
						
						for (const auto& Itr : CollisionComponents)
						{
							Execute_UnbindCollisionShape(this, Itr);
						}
					}
					break;
				case EInteractableStateV2::EIS_Completed:
				case EInteractableStateV2::EIS_Asleep:
				case EInteractableStateV2::Default: 
				default: break;
			}
			break;
		case EInteractableStateV2::EIS_Cooldown:
			switch (InteractableState)
			{
				case EInteractableStateV2::EIS_Awake:
				case EInteractableStateV2::EIS_Active:
					InteractableState = NewState;
					Execute_StopHighlight(this);
					OnInteractableStateChanged.Broadcast(InteractableState);
					break;
				case EInteractableStateV2::EIS_Suppressed:
				case EInteractableStateV2::EIS_Disabled:
					{
						InteractableState = NewState;

						// Replacing Cleanup
						Execute_StopHighlight(this);
						OnInteractableStateChanged.Broadcast(InteractableState);
						if (GetWorld()) GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
						OnInteractorLost.Broadcast(Interactor);
						
						for (const auto& Itr : CollisionComponents)
						{
							Execute_UnbindCollisionShape(this, Itr);
						}
					}
					break;
				case EInteractableStateV2::EIS_Paused:
				case EInteractableStateV2::EIS_Cooldown:
				case EInteractableStateV2::EIS_Completed:
				case EInteractableStateV2::EIS_Asleep:
				case EInteractableStateV2::Default: 
				default: break;
			}
			break;
		case EInteractableStateV2::EIS_Completed:
			switch (InteractableState)
			{
				case EInteractableStateV2::EIS_Active:
					{
						InteractableState = NewState;
						CleanupComponent();
					}
					break;
				case EInteractableStateV2::EIS_Completed:
				case EInteractableStateV2::EIS_Paused:
				case EInteractableStateV2::EIS_Suppressed:
				case EInteractableStateV2::EIS_Awake:
				case EInteractableStateV2::EIS_Cooldown:
				case EInteractableStateV2::EIS_Disabled:
				case EInteractableStateV2::EIS_Asleep:
				case EInteractableStateV2::Default: 
				default: break;
			}
			break;
		case EInteractableStateV2::EIS_Disabled:
			switch (InteractableState)
			{
				case EInteractableStateV2::EIS_Active:
				case EInteractableStateV2::EIS_Paused:
				case EInteractableStateV2::EIS_Completed:
				case EInteractableStateV2::EIS_Awake:
				case EInteractableStateV2::EIS_Suppressed:
				case EInteractableStateV2::EIS_Cooldown:
				case EInteractableStateV2::EIS_Asleep:
					{
						InteractableState = NewState;

						// Replacing Cleanup
						Execute_StopHighlight(this);
						OnInteractableStateChanged.Broadcast(InteractableState);
						if (GetWorld()) GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
						OnInteractorLost.Broadcast(Interactor);
						
						for (const auto& Itr : CollisionComponents)
						{
							Execute_UnbindCollisionShape(this, Itr);
						}
					}
					break;
				case EInteractableStateV2::EIS_Disabled:
				case EInteractableStateV2::Default: 
				default: break;
			}
			break;
		case EInteractableStateV2::EIS_Suppressed:
			switch (InteractableState)
			{
				case EInteractableStateV2::EIS_Active:
				case EInteractableStateV2::EIS_Awake:
				case EInteractableStateV2::EIS_Asleep:
				case EInteractableStateV2::EIS_Disabled:
				case EInteractableStateV2::EIS_Paused:
					OnInteractionCanceled.Broadcast();
					InteractableState = NewState;
					Execute_StopHighlight(this);
					OnInteractableStateChanged.Broadcast(InteractableState);
					break;
				case EInteractableStateV2::EIS_Cooldown:
					OnInteractionCanceled.Broadcast();
					InteractableState = NewState;
					Execute_StopHighlight(this);
					OnInteractableStateChanged.Broadcast(InteractableState);
					GetWorld()->GetTimerManager().ClearTimer(Timer_Cooldown);
					break;
				case EInteractableStateV2::EIS_Completed:
				case EInteractableStateV2::EIS_Suppressed:
				case EInteractableStateV2::Default:
				default: break;
			}
			break;
		case EInteractableStateV2::EIS_Paused:
			switch (InteractableState)
			{
				case EInteractableStateV2::EIS_Active:
					{
						InteractableState = NewState;
						OnInteractableStateChanged.Broadcast(InteractableState);
						break;
					}
				case EInteractableStateV2::EIS_Paused:
				case EInteractableStateV2::EIS_Awake:
				case EInteractableStateV2::EIS_Asleep:
				case EInteractableStateV2::EIS_Disabled:
				case EInteractableStateV2::EIS_Cooldown:
				case EInteractableStateV2::EIS_Completed:
				case EInteractableStateV2::EIS_Suppressed:
				case EInteractableStateV2::Default:
				default: break;
			}
			break;
		case EInteractableStateV2::Default: 
		default:
			Execute_StopHighlight(this);
			break;
	}
	
	Execute_ProcessDependencies(this);
}

void UActorInteractableComponentBase::StartHighlight_Implementation()
{
	SetHiddenInGame(false, true);
	switch (HighlightType)
	{
		case EHighlightType::EHT_PostProcessing:
			{
				for (const auto Itr : HighlightableComponents)
				{
					Itr->SetRenderCustomDepth(bInteractionHighlight);
					Itr->SetCustomDepthStencilValue(StencilID);
				}
			}
			break;
		case EHighlightType::EHT_OverlayMaterial:
			{
				for (const auto Itr : HighlightableComponents)
				{
					Itr->SetOverlayMaterial(HighlightMaterial);
				}
			}
		case EHighlightType::EHT_Default:
		default:
			break;
	}
}

void UActorInteractableComponentBase::StopHighlight_Implementation()
{
	SetHiddenInGame(true, true);
	switch (HighlightType)
	{
		case EHighlightType::EHT_PostProcessing:
			{
				for (const auto Itr : HighlightableComponents)
				{
					Itr->SetCustomDepthStencilValue(0);
				}
			}
			break;
		case EHighlightType::EHT_OverlayMaterial:
			{
				for (const auto Itr : HighlightableComponents)
				{
					Itr->SetOverlayMaterial(nullptr);
				}
			}
		case EHighlightType::EHT_Default:
		default:
			break;
	}
}

TArray<TSoftClassPtr<UObject>> UActorInteractableComponentBase::GetIgnoredClasses_Implementation() const
{ return IgnoredClasses; }

void UActorInteractableComponentBase::SetIgnoredClasses_Implementation(const TArray<TSoftClassPtr<UObject>>& NewIgnoredClasses)
{
	IgnoredClasses.Empty();

	IgnoredClasses = NewIgnoredClasses;
}

void UActorInteractableComponentBase::AddIgnoredClass_Implementation(const TSoftClassPtr<UObject>& AddIgnoredClass)
{
	if (AddIgnoredClass == nullptr) return;

	if (IgnoredClasses.Contains(AddIgnoredClass)) return;

	IgnoredClasses.Add(AddIgnoredClass);

	OnIgnoredInteractorClassAdded.Broadcast(AddIgnoredClass);
}

void UActorInteractableComponentBase::AddIgnoredClasses_Implementation(const TArray<TSoftClassPtr<UObject>>& AddIgnoredClasses)
{
	for (const auto& Itr : AddIgnoredClasses)
	{
		Execute_AddIgnoredClass(this, Itr);
	}
}

void UActorInteractableComponentBase::RemoveIgnoredClass_Implementation(const TSoftClassPtr<UObject>& RemoveIgnoredClass)
{
	if (RemoveIgnoredClass == nullptr) return;

	if (!IgnoredClasses.Contains(RemoveIgnoredClass)) return;

	IgnoredClasses.Remove(RemoveIgnoredClass);

	OnIgnoredInteractorClassRemoved.Broadcast(RemoveIgnoredClass);
}

void UActorInteractableComponentBase::RemoveIgnoredClasses_Implementation(const TArray<TSoftClassPtr<UObject>>& RemoveIgnoredClasses)
{
	for (const auto& Itr : RemoveIgnoredClasses)
	{
		Execute_RemoveIgnoredClass(this, Itr);
	}
}

void UActorInteractableComponentBase::AddInteractionDependency_Implementation(const TScriptInterface<IActorInteractableInterface>& InteractionDependency)
{
	if (InteractionDependency.GetObject() == nullptr) return;
	if (InteractionDependencies.Contains(InteractionDependency)) return;

	OnInteractableDependencyChanged.Broadcast(InteractionDependency);
	
	InteractionDependencies.Add(InteractionDependency);

	InteractionDependency->GetInteractableDependencyStarted().Broadcast(this);
}

void UActorInteractableComponentBase::RemoveInteractionDependency_Implementation(const TScriptInterface<IActorInteractableInterface>& InteractionDependency)
{
	if (InteractionDependency.GetObject() == nullptr) return;
	if (!InteractionDependencies.Contains(InteractionDependency)) return;

	OnInteractableDependencyChanged.Broadcast(InteractionDependency);

	InteractionDependencies.Remove(InteractionDependency);

	InteractionDependency->GetInteractableDependencyStopped().Broadcast(this);
}

TArray<TScriptInterface<IActorInteractableInterface>> UActorInteractableComponentBase::GetInteractionDependencies_Implementation() const
{ return InteractionDependencies; }

void UActorInteractableComponentBase::ProcessDependencies_Implementation()
{
	if (InteractionDependencies.Num() == 0) return;

	auto Dependencies = InteractionDependencies;
	for (const auto& Itr : Dependencies)
	{
		switch (InteractableState)
		{
			case EInteractableStateV2::EIS_Active:
			case EInteractableStateV2::EIS_Suppressed:
				Itr->GetInteractableDependencyStarted().Broadcast(this);
				switch (Itr->Execute_GetState(Itr.GetObject()))
				{
					case EInteractableStateV2::EIS_Active:
					case EInteractableStateV2::EIS_Awake:
					case EInteractableStateV2::EIS_Asleep:
						Itr->Execute_SetState(this, EInteractableStateV2::EIS_Suppressed);
						break;
					case EInteractableStateV2::EIS_Cooldown:
						
						Itr->Execute_SetState(this, EInteractableStateV2::EIS_Suppressed);
						
						break;
					case EInteractableStateV2::EIS_Completed: break;
					case EInteractableStateV2::EIS_Disabled: break;
					case EInteractableStateV2::EIS_Suppressed: break;
					case EInteractableStateV2::Default: break;
					default: break;
				}
				break;
			case EInteractableStateV2::EIS_Cooldown:
			case EInteractableStateV2::EIS_Awake:
			case EInteractableStateV2::EIS_Asleep:
				Itr->GetInteractableDependencyStarted().Broadcast(this);
				switch (Itr->Execute_GetState(Itr.GetObject()))
				{
					
					case EInteractableStateV2::EIS_Awake:
					case EInteractableStateV2::EIS_Asleep:
					case EInteractableStateV2::EIS_Suppressed: 
						Itr->Execute_SetState(this, Itr->Execute_GetDefaultState(Itr.GetObject()));
						break;
					case EInteractableStateV2::EIS_Cooldown: break;
					case EInteractableStateV2::EIS_Completed: break;
					case EInteractableStateV2::EIS_Disabled: break;
					case EInteractableStateV2::EIS_Active:
					case EInteractableStateV2::Default: break;
					default: break;
				}
				break;
			case EInteractableStateV2::EIS_Disabled:
			case EInteractableStateV2::EIS_Completed:
				Itr->GetInteractableDependencyStopped().Broadcast(this);
				Itr->Execute_SetState(this, Itr->Execute_GetDefaultState(Itr.GetObject()));
				Execute_RemoveInteractionDependency(this, Itr);
				break;
			case EInteractableStateV2::Default:
			default:
				break;
		}
	}
}

TScriptInterface<IActorInteractorInterface> UActorInteractableComponentBase::GetInteractor_Implementation() const
{ return Interactor; }

void UActorInteractableComponentBase::SetInteractor_Implementation(const TScriptInterface<IActorInteractorInterface>& NewInteractor)
{
	const TScriptInterface<IActorInteractorInterface> OldInteractor = Interactor;

	Interactor = NewInteractor;
	
	if (NewInteractor.GetInterface() != nullptr)
	{
		NewInteractor->GetOnInteractableSelectedHandle().AddUniqueDynamic(this, &UActorInteractableComponentBase::InteractableSelected);
		NewInteractor->GetOnInteractableFoundHandle().Broadcast(this);
	}
	else
	{
		if (OldInteractor.GetInterface() != nullptr)
		{
			OldInteractor->GetOnInteractableSelectedHandle().RemoveDynamic(this, &UActorInteractableComponentBase::InteractableSelected);
		}

		Execute_StopHighlight(this);
	}

	//Interactor = NewInteractor;
	OnInteractorChanged.Broadcast(Interactor);
}

float UActorInteractableComponentBase::GetInteractionProgress_Implementation() const
{
	if (!GetWorld()) return -1;

	if (Timer_Interaction.IsValid())
	{
		return GetWorld()->GetTimerManager().GetTimerElapsed(Timer_Interaction) / InteractionPeriod;
	}
	return 0.f;
}

float UActorInteractableComponentBase::GetInteractionPeriod_Implementation() const
{ return InteractionPeriod; }

void UActorInteractableComponentBase::SetInteractionPeriod_Implementation(const float NewPeriod)
{
	float TempPeriod = NewPeriod;
	if (TempPeriod > -1.f && TempPeriod < 0.01f)
	{
		TempPeriod = 0.01f;
	}
	if (FMath::IsNearlyZero(TempPeriod, 0.0001f))
	{
		TempPeriod = 0.01f;
	}

	InteractionPeriod = FMath::Max(-1.f, TempPeriod);
}

int32 UActorInteractableComponentBase::GetInteractableWeight_Implementation() const
{ return InteractionWeight; }

void UActorInteractableComponentBase::SetInteractableWeight_Implementation(const int32 NewWeight)
{
	InteractionWeight = NewWeight;

	OnInteractableWeightChanged.Broadcast(InteractionWeight);
}

AActor* UActorInteractableComponentBase::GetInteractableOwner_Implementation() const
{ return GetOwner(); }

ECollisionChannel UActorInteractableComponentBase::GetCollisionChannel_Implementation() const
{ return CollisionChannel; }

void UActorInteractableComponentBase::SetCollisionChannel_Implementation(const TEnumAsByte<ECollisionChannel>& NewChannel)
{
	CollisionChannel = NewChannel;

	OnInteractableCollisionChannelChanged.Broadcast(CollisionChannel);
}

TArray<UPrimitiveComponent*> UActorInteractableComponentBase::GetCollisionComponents_Implementation() const
{	return CollisionComponents;}

EInteractableLifecycle UActorInteractableComponentBase::GetLifecycleMode_Implementation() const
{	return LifecycleMode;}

void UActorInteractableComponentBase::SetLifecycleMode_Implementation(const EInteractableLifecycle& NewMode)
{
	LifecycleMode = NewMode;

	OnLifecycleModeChanged.Broadcast(LifecycleMode);
}

int32 UActorInteractableComponentBase::GetLifecycleCount_Implementation() const
{	return LifecycleCount;}

void UActorInteractableComponentBase::SetLifecycleCount_Implementation(const int32 NewLifecycleCount)
{
	switch (LifecycleMode)
	{
		case EInteractableLifecycle::EIL_Cycled:
			if (NewLifecycleCount < -1)
			{
				LifecycleCount = -1;
				OnLifecycleCountChanged.Broadcast(LifecycleCount);
			}
			else if (NewLifecycleCount < 2)
			{
				LifecycleCount = 2;
				OnLifecycleCountChanged.Broadcast(LifecycleCount);
			}
			else if (NewLifecycleCount > 2)
			{
				LifecycleCount = NewLifecycleCount;
				OnLifecycleCountChanged.Broadcast(LifecycleCount);
			}
			break;
		case EInteractableLifecycle::EIL_OnlyOnce:
		case EInteractableLifecycle::Default:
		default: break;
	}
}

int32 UActorInteractableComponentBase::GetRemainingLifecycleCount_Implementation() const
{ return RemainingLifecycleCount; }

float UActorInteractableComponentBase::GetCooldownPeriod_Implementation() const
{ return CooldownPeriod; }

void UActorInteractableComponentBase::SetCooldownPeriod_Implementation(const float NewCooldownPeriod)
{
	switch (LifecycleMode)
	{
		case EInteractableLifecycle::EIL_Cycled:
			LifecycleCount = FMath::Max(0.1f, NewCooldownPeriod);
			OnLifecycleCountChanged.Broadcast(LifecycleCount);
			break;
		case EInteractableLifecycle::EIL_OnlyOnce:
		case EInteractableLifecycle::Default:
		default: break;
	}
}

void UActorInteractableComponentBase::AddCollisionComponent_Implementation(UPrimitiveComponent* CollisionComp)
{
	if (CollisionComp == nullptr) return;
	if (CollisionComponents.Contains(CollisionComp)) return;
	
	CollisionComponents.Add(CollisionComp);
	
	Execute_BindCollisionShape(this, CollisionComp);
	
	OnCollisionComponentAdded.Broadcast(CollisionComp);
}

void UActorInteractableComponentBase::AddCollisionComponents_Implementation(const TArray<UPrimitiveComponent*>& NewCollisionComponents)
{
	for (UPrimitiveComponent* const Itr : NewCollisionComponents)
	{
		Execute_AddCollisionComponent(this, Itr);
	}
}

void UActorInteractableComponentBase::RemoveCollisionComponent_Implementation(UPrimitiveComponent* CollisionComp)
{
	if (CollisionComp == nullptr) return;
	if (!CollisionComponents.Contains(CollisionComp)) return;
	
	CollisionComponents.Remove(CollisionComp);

	Execute_UnbindCollisionShape(this, CollisionComp);
	
	OnCollisionComponentRemoved.Broadcast(CollisionComp);
}

void UActorInteractableComponentBase::RemoveCollisionComponents_Implementation(const TArray<UPrimitiveComponent*>& RemoveCollisionComponents)
{
	for (UPrimitiveComponent* const Itr : RemoveCollisionComponents)
	{
		Execute_RemoveCollisionComponent(this, Itr);
	}
}

TArray<UMeshComponent*> UActorInteractableComponentBase::GetHighlightableComponents_Implementation() const
{	return HighlightableComponents;}

void UActorInteractableComponentBase::AddHighlightableComponent_Implementation(UMeshComponent* MeshComponent)
{
	if (MeshComponent == nullptr) return;
	if (HighlightableComponents.Contains(MeshComponent)) return;

	HighlightableComponents.Add(MeshComponent);

	Execute_BindHighlightableMesh(this, MeshComponent);

	OnHighlightableComponentAdded.Broadcast(MeshComponent);
}

void UActorInteractableComponentBase::AddHighlightableComponents_Implementation(const TArray<UMeshComponent*>& AddMeshComponents)
{
	for (UMeshComponent* const Itr : HighlightableComponents)
	{
		Execute_AddHighlightableComponent(this, Itr);
	}
}

void UActorInteractableComponentBase::RemoveHighlightableComponent_Implementation(UMeshComponent* MeshComponent)
{
	if (MeshComponent == nullptr) return;
	if (!HighlightableComponents.Contains(MeshComponent)) return;

	HighlightableComponents.Remove(MeshComponent);

	Execute_UnbindHighlightableMesh(this, MeshComponent);

	OnHighlightableComponentRemoved.Broadcast(MeshComponent);
}

void UActorInteractableComponentBase::RemoveHighlightableComponents_Implementation(const TArray<UMeshComponent*>& RemoveMeshComponents)
{
	for (UMeshComponent* const Itr : RemoveMeshComponents)
	{
		Execute_RemoveHighlightableComponent(this, Itr);
	}
}

TArray<FName> UActorInteractableComponentBase::GetCollisionOverrides_Implementation() const
{	return CollisionOverrides;}

TArray<FName> UActorInteractableComponentBase::GetHighlightableOverrides_Implementation() const
{	return HighlightableOverrides;}

FDataTableRowHandle UActorInteractableComponentBase::GetInteractableData_Implementation() const
{ return InteractableData; }

void UActorInteractableComponentBase::SetInteractableData_Implementation(FDataTableRowHandle NewData)
{ InteractableData = NewData; }

FText UActorInteractableComponentBase::GetInteractableName_Implementation() const
{ return InteractableName; }

void UActorInteractableComponentBase::SetInteractableName_Implementation(const FText& NewName)
{
	if (NewName.IsEmpty()) return;
	InteractableName = NewName;
}

EHighlightType UActorInteractableComponentBase::GetHighlightType_Implementation() const
{ return HighlightType; }

void UActorInteractableComponentBase::SetHighlightType_Implementation(const EHighlightType NewHighlightType)
{
	HighlightType = NewHighlightType;

	OnHighlightTypeChanged.Broadcast(NewHighlightType);
}

UMaterialInterface* UActorInteractableComponentBase::GetHighlightMaterial_Implementation() const
{ return HighlightMaterial; }

void UActorInteractableComponentBase::SetHighlightMaterial_Implementation(UMaterialInterface* NewHighlightMaterial)
{
	HighlightMaterial = NewHighlightMaterial;

	OnHighlightMaterialChanged.Broadcast(NewHighlightMaterial);
}

ETimingComparison UActorInteractableComponentBase::GetComparisonMethod_Implementation() const
{ return ComparisonMethod; }

void UActorInteractableComponentBase::SetComparisonMethod_Implementation(const ETimingComparison Value)
{ ComparisonMethod = Value; }

void UActorInteractableComponentBase::SetDefaults_Implementation()
{
	if (const auto DefaultTable = UActorInteractionFunctionLibrary::GetInteractableDefaultDataTable())
	{
		InteractableData.DataTable = DefaultTable;
	}
	
	if (const auto DefaultWidgetClass = UActorInteractionFunctionLibrary::GetInteractableDefaultWidgetClass())
	{
		if (DefaultWidgetClass != nullptr)
		{
			SetWidgetClass(DefaultWidgetClass.Get());
		}
	}
}

FGameplayTagContainer UActorInteractableComponentBase::GetInteractableCompatibleTags_Implementation() const
{
	return InteractableCompatibleTags;
}

void UActorInteractableComponentBase::SetInteractableCompatibleTags_Implementation(const FGameplayTagContainer& Tags)
{
	InteractableCompatibleTags = Tags;
}

void UActorInteractableComponentBase::AddInteractableCompatibleTag_Implementation(const FGameplayTag& Tag)
{
	InteractableCompatibleTags.AddTag(Tag);
}

void UActorInteractableComponentBase::AddInteractableCompatibleTags_Implementation(const FGameplayTagContainer& Tags)
{
	InteractableCompatibleTags.AppendTags(Tags);
}

void UActorInteractableComponentBase::RemoveInteractableCompatibleTag_Implementation(const FGameplayTag& Tag)
{
	InteractableCompatibleTags.RemoveTag(Tag);
}

void UActorInteractableComponentBase::RemoveInteractableCompatibleTags_Implementation(const FGameplayTagContainer& Tags)
{
	InteractableCompatibleTags.RemoveTags(Tags);
}

void UActorInteractableComponentBase::ClearInteractableCompatibleTags_Implementation()
{
	InteractableCompatibleTags.Reset();
}

void UActorInteractableComponentBase::InteractorFound_Implementation(const TScriptInterface<IActorInteractorInterface>& FoundInteractor)
{
	if (Execute_CanBeTriggered(this))
	{
		Execute_ToggleWidgetVisibility(this, true);
		
		Execute_SetInteractor(this, FoundInteractor);
		
		Execute_OnInteractorFoundEvent(this, FoundInteractor);
	}
}

void UActorInteractableComponentBase::InteractorLost_Implementation(const TScriptInterface<IActorInteractorInterface>& LostInteractor)
{
	if (LostInteractor.GetInterface() == nullptr) return;
	
	if (Interactor == LostInteractor)
	{
		GetWorld()->GetTimerManager().ClearTimer(Timer_Interaction);
		GetWorld()->GetTimerManager().ClearTimer(Timer_ProgressExpiration);
		
		Execute_ToggleWidgetVisibility(this, false);

		switch (InteractableState)
		{
			case EInteractableStateV2::EIS_Cooldown:
				if (Execute_GetInteractor(this).GetObject() == nullptr)
				{
					Execute_SetState(this, DefaultInteractableState);
				}
				break;
			case EInteractableStateV2::EIS_Asleep:
			case EInteractableStateV2::EIS_Suppressed:
				Execute_SetState(this, DefaultInteractableState);
				break;
			case EInteractableStateV2::EIS_Active:
			case EInteractableStateV2::EIS_Awake:
			case EInteractableStateV2::EIS_Paused:
				Execute_SetState(this, DefaultInteractableState);
				break;
			case EInteractableStateV2::EIS_Completed:
			case EInteractableStateV2::EIS_Disabled:
			case EInteractableStateV2::Default:
			default: break;
		}
		
		if (Interactor.GetInterface() != nullptr)
		{
			Interactor->GetOnInteractableSelectedHandle().RemoveDynamic(this, &UActorInteractableComponentBase::InteractableSelected);
			Interactor->GetOnInteractableLostHandle().RemoveDynamic(this, &UActorInteractableComponentBase::InteractableLost);
		}
		
		Execute_SetInteractor(this, nullptr);
		Execute_OnInteractorLostEvent(this, LostInteractor);

		OnInteractionCanceled.Broadcast();
	}
}

void UActorInteractableComponentBase::InteractionCompleted_Implementation(const float& TimeCompleted, const TScriptInterface<IActorInteractorInterface>& CausingInteractor)
{
	Execute_ToggleWidgetVisibility(this, false);
	
	if (LifecycleMode == EInteractableLifecycle::EIL_Cycled)
	{
		if (Execute_TriggerCooldown(this)) return;
	}
	
	FString ErrorMessage;
	if( Execute_CompleteInteractable(this, ErrorMessage))
	{
		Execute_OnInteractionCompletedEvent(this, TimeCompleted, CausingInteractor);
	}
	else LOG_INFO(TEXT("%s"), *ErrorMessage);
}

void UActorInteractableComponentBase::InteractionCycleCompleted_Implementation(const float& CompletedTime, const int32 CyclesRemaining, const TScriptInterface<IActorInteractorInterface>& CausingInteractor)
{
	Execute_OnInteractionCycleCompletedEvent(this, CompletedTime, CyclesRemaining, CausingInteractor);
}

void UActorInteractableComponentBase::InteractionStarted_Implementation(const float& TimeStarted, const TScriptInterface<IActorInteractorInterface>& CausingInteractor)
{
	if (Execute_CanInteract(this))
	{
		GetWorld()->GetTimerManager().ClearTimer(Timer_ProgressExpiration);
		
		Execute_SetState(this, EInteractableStateV2::EIS_Active);
		Execute_OnInteractionStartedEvent(this, TimeStarted, CausingInteractor);
	}
}

void UActorInteractableComponentBase::InteractionStopped_Implementation(const float& TimeStarted, const TScriptInterface<IActorInteractorInterface>& CausingInteractor)
{
	if (!GetWorld()) return;

	if (bCanPersist)
	{
		Execute_PauseInteraction(this, InteractionProgressExpiration, CausingInteractor);
	}
	else
	{
		Execute_InteractionCanceled(this);
	}
}

void UActorInteractableComponentBase::InteractionCanceled_Implementation()
{
	if (Execute_CanInteract(this))
	{
		Execute_ToggleWidgetVisibility(this, false);
		
		GetWorld()->GetTimerManager().ClearTimer(Timer_Interaction);
		GetWorld()->GetTimerManager().ClearTimer(Timer_ProgressExpiration);
		
		switch (InteractableState)
		{
			case EInteractableStateV2::EIS_Cooldown:
			case EInteractableStateV2::EIS_Awake:
				if (Execute_GetInteractor(this).GetObject() == nullptr)
				{
					Execute_SetState(this, DefaultInteractableState);
				}
				break;
			case EInteractableStateV2::EIS_Active:
			case EInteractableStateV2::EIS_Asleep:
			case EInteractableStateV2::EIS_Completed:
			case EInteractableStateV2::EIS_Disabled:
			case EInteractableStateV2::EIS_Suppressed:
				Execute_SetState(this, DefaultInteractableState);
				break;
			case EInteractableStateV2::Default:
			default: break;
		}
		
		Execute_OnInteractionCanceledEvent(this);
	}
}

void UActorInteractableComponentBase::InteractionLifecycleCompleted_Implementation()
{
	Execute_SetState(this, EInteractableStateV2::EIS_Completed);

	Execute_OnLifecycleCompletedEvent(this);
}

void UActorInteractableComponentBase::InteractionCooldownCompleted_Implementation()
{
	if (Interactor.GetInterface() != nullptr)
	{
		Execute_StartHighlight(this);

		Execute_SetState(this, DefaultInteractableState);
		
		if (Interactor->Execute_GetActiveInteractable(Interactor.GetObject()) == this)
		{
			Execute_SetState(this, EInteractableStateV2::EIS_Active);
		}
		else
		{
			Execute_StopHighlight(this);
			Execute_SetState(this, DefaultInteractableState);
		}
	}
	else
	{
		Execute_StopHighlight(this);
		Execute_SetState(this, DefaultInteractableState);
	}
	
	Execute_OnCooldownCompletedEvent(this);
}

void UActorInteractableComponentBase::OnInteractableBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	/*
	if (!Execute_CanBeTriggered(this)) return;
	if (Execute_IsInteracting(this)) return;
	if (!OtherActor) return;
	if (!OtherComp) return;

	if (OtherComp->GetCollisionResponseToChannel(CollisionChannel) == ECollisionResponse::ECR_Ignore) return;

	TArray<UActorComponent*> InteractorComponents = OtherActor->GetComponentsByInterface(UActorInteractorInterface::StaticClass());

	if (InteractorComponents.Num() == 0) return;
	
	for (const auto& Itr : InteractorComponents)
	{
		TScriptInterface<IActorInteractorInterface> FoundInteractor;
		if (IgnoredClasses.Contains(Itr->StaticClass())) continue;
		
		FoundInteractor = Itr;
		FoundInteractor.SetObject(Itr);
		FoundInteractor.SetInterface(Cast<IActorInteractorInterface>(Itr));

		if (FoundInteractor->Execute_CanInteract(FoundInteractor.GetObject()))
		{
			switch (FoundInteractor->Execute_GetState(FoundInteractor.GetObject()))
			{
				case EInteractorStateV2::EIS_Active:
				case EInteractorStateV2::EIS_Awake:
					if (FoundInteractor->Execute_GetResponseChannel(FoundInteractor.GetObject()) != Execute_GetCollisionChannel(this)) continue;
					FoundInteractor->GetOnInteractableLostHandle().AddUniqueDynamic(this, &UActorInteractableComponentBase::InteractableLost);
					FoundInteractor->GetOnInteractableSelectedHandle().AddUniqueDynamic(this, &UActorInteractableComponentBase::InteractableSelected);
					OnInteractorFound.Broadcast(FoundInteractor);
					OnInteractorOverlapped.Broadcast(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
					break;
				case EInteractorStateV2::EIS_Asleep:
				case EInteractorStateV2::EIS_Suppressed:
				case EInteractorStateV2::EIS_Disabled:
				case EInteractorStateV2::Default:
					break;
			}
		}
	}
	*/
}

void UActorInteractableComponentBase::OnInteractableStopOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	/*
	if (!OtherActor) return;

	TArray<UActorComponent*> InteractorComponents = OtherActor->GetComponentsByInterface(UActorInteractorInterface::StaticClass());

	if (InteractorComponents.Num() == 0) return;

	for (const auto& Itr : InteractorComponents)
	{
		TScriptInterface<IActorInteractorInterface> LostInteractor;
		if (IgnoredClasses.Contains(Itr->StaticClass())) continue;
		
		LostInteractor = Itr;
		LostInteractor.SetObject(Itr);
		LostInteractor.SetInterface(Cast<IActorInteractorInterface>(Itr));

		if (LostInteractor->Execute_CanInteract(LostInteractor.GetObject()))
		{
			if (LostInteractor == Execute_GetInteractor(this))
			{
				Execute_GetInteractor(this)->GetOnInteractableLostHandle().RemoveDynamic(this, &UActorInteractableComponentBase::InteractableLost);
				Execute_GetInteractor(this)->GetOnInteractableLostHandle().Broadcast(this);
				
				OnInteractorLost.Broadcast(Execute_GetInteractor(this));
				OnInteractorStopOverlap.Broadcast(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
				
				return;
			}
		}
	}
	*/
}

void UActorInteractableComponentBase::OnInteractableTraced(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	/*
	if (!Execute_CanBeTriggered(this)) return;
	if (!OtherActor) return;

	TArray<UActorComponent*> InteractorComponents = OtherActor->GetComponentsByInterface(UActorInteractorInterface::StaticClass());

	if (InteractorComponents.Num() == 0) return;
	
	for (const auto& Itr : InteractorComponents)
	{
		TScriptInterface<IActorInteractorInterface> FoundInteractor;
		if (IgnoredClasses.Contains(Itr->StaticClass())) continue;
		
		FoundInteractor = Itr;
		FoundInteractor.SetObject(Itr);
		FoundInteractor.SetInterface(Cast<IActorInteractorInterface>(Itr));

		switch (FoundInteractor->Execute_GetState(FoundInteractor.GetObject()))
		{
			case EInteractorStateV2::EIS_Active:
			case EInteractorStateV2::EIS_Awake:
				if (FoundInteractor->Execute_CanInteract(FoundInteractor.GetObject()) == false) return;
				if (FoundInteractor->Execute_GetResponseChannel(FoundInteractor.GetObject()) != Execute_GetCollisionChannel(this)) continue;
				FoundInteractor->GetOnInteractableLostHandle().AddUniqueDynamic(this, &UActorInteractableComponentBase::InteractableLost);
				FoundInteractor->GetOnInteractableSelectedHandle().AddUniqueDynamic(this, &UActorInteractableComponentBase::InteractableSelected);
				OnInteractorFound.Broadcast(FoundInteractor);
				Execute_OnInteractableTracedEvent(this, HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
				break;
			case EInteractorStateV2::EIS_Asleep:
			case EInteractorStateV2::EIS_Suppressed:
			case EInteractorStateV2::EIS_Disabled:
			case EInteractorStateV2::Default:
				break;
		}
	}
	*/
}

void UActorInteractableComponentBase::OnInteractionProgressExpired(const float ExpirationTime, const TScriptInterface<IActorInteractorInterface>& CausingInteractor)
{
	if (!GetWorld()) return;
	
	switch (InteractableState)
	{
		case EInteractableStateV2::EIS_Paused:
			{
				GetWorld()->GetTimerManager().ClearTimer(Timer_Interaction);
				GetWorld()->GetTimerManager().ClearTimer(Timer_ProgressExpiration);
				
				auto localInteractor = Execute_GetInteractor(this);
				if (Execute_DoesHaveInteractor(this) && localInteractor.GetObject() && localInteractor->Execute_GetActiveInteractable(localInteractor.GetObject()) == this)
				{
					Execute_SetState(this, EInteractableStateV2::EIS_Active);
				}
				else
				{
					Execute_OnInteractionStoppedEvent(this, ExpirationTime, CausingInteractor);
				}
			}
			break;
		case EInteractableStateV2::EIS_Active: break;
		case EInteractableStateV2::EIS_Awake: break;
		case EInteractableStateV2::EIS_Cooldown: break;
		case EInteractableStateV2::EIS_Completed: break;
		case EInteractableStateV2::EIS_Disabled: break;
		case EInteractableStateV2::EIS_Suppressed: break;
		case EInteractableStateV2::EIS_Asleep: break;
		case EInteractableStateV2::Default: break;
	}
}

void UActorInteractableComponentBase::InteractableComponentActivated(UActorComponent* Component, bool bReset)
{
	if (IsActive())
	{
		AutoSetup();
	}
}

void UActorInteractableComponentBase::InteractableSelected_Implementation(const TScriptInterface<IActorInteractableInterface>& Interactable)
{
 	if (Interactable == this)
 	{
 		Execute_StartHighlight(this);
 		
 		Execute_SetState(this, EInteractableStateV2::EIS_Active);
 		OnInteractableSelected.Broadcast(Interactable);
 	}
	else
	{
		OnInteractionCanceled.Broadcast();
		
		Execute_SetState(this, DefaultInteractableState);
		OnInteractorLost.Broadcast(Execute_GetInteractor(this));
	}
}

void UActorInteractableComponentBase::InteractableLost_Implementation(const TScriptInterface<IActorInteractableInterface>& Interactable)
{
	if (Interactable == this)
	{
		switch (InteractableState)
		{
			case EInteractableStateV2::EIS_Active:
				Execute_SetState(this, DefaultInteractableState);
				break;
			case EInteractableStateV2::EIS_Cooldown:
			case EInteractableStateV2::EIS_Awake:
				if (Execute_GetInteractor(this).GetObject() == nullptr)
				{
					Execute_SetState(this, DefaultInteractableState);
				}
				break;
			case EInteractableStateV2::EIS_Asleep:
			case EInteractableStateV2::EIS_Completed:
			case EInteractableStateV2::EIS_Disabled:
			case EInteractableStateV2::EIS_Suppressed:
			case EInteractableStateV2::Default:
			default: break;
		}
		
		OnInteractorLost.Broadcast(Execute_GetInteractor(this));
	}
}

void UActorInteractableComponentBase::FindAndAddCollisionShapes_Implementation()
{
	for (const auto& Itr : CollisionOverrides)
	{
		if (const auto NewCollision = UMounteaInteractionSystemBFL::FindPrimitiveByName(Itr, GetOwner()))
		{
			Execute_AddCollisionComponent(this, NewCollision);
			Execute_BindCollisionShape(this, NewCollision);
		}
		else
		{
			if (const auto NewCollisionByTag = UMounteaInteractionSystemBFL::FindPrimitiveByTag(Itr, GetOwner()))
			{
				Execute_AddCollisionComponent(this, NewCollisionByTag);
				Execute_BindCollisionShape(this, NewCollisionByTag);
			}
			else LOG_ERROR(TEXT("[Actor Interactable Component] Primitive Component '%s' not found!"), *Itr.ToString())
		}
	}
}

void UActorInteractableComponentBase::FindAndAddHighlightableMeshes_Implementation()
{
	for (const auto& Itr : HighlightableOverrides)
	{
		if (const auto NewMesh = UMounteaInteractionSystemBFL::FindMeshByName(Itr, GetOwner()))
		{
			Execute_AddHighlightableComponent(this, NewMesh);
			Execute_BindHighlightableMesh(this, NewMesh);
		}
		else
		{
			if (const auto NewHighlightByTag = UMounteaInteractionSystemBFL::FindMeshByTag(Itr, GetOwner()))
			{
				Execute_AddHighlightableComponent(this, NewHighlightByTag);
				Execute_BindHighlightableMesh(this, NewHighlightByTag);
			}
			else LOG_ERROR(TEXT("[Actor Interactable Component] Mesh Component '%s' not found!"), *Itr.ToString())
		}
	}
}

bool UActorInteractableComponentBase::TriggerCooldown_Implementation()
{
	if (LifecycleCount != -1)
	{
		const int32 TempRemainingLifecycleCount = RemainingLifecycleCount - 1;
		RemainingLifecycleCount = FMath::Max(0, TempRemainingLifecycleCount);
	}
	
	if (GetWorld())
	{
		if (RemainingLifecycleCount == 0) return false;

		Execute_SetState(this, EInteractableStateV2::EIS_Cooldown);

		FTimerDelegate Delegate;
		Delegate.BindUObject(this, &UActorInteractableComponentBase::OnCooldownCompletedCallback);
		
		GetWorld()->GetTimerManager().SetTimer
		(
			Timer_Cooldown,
			Delegate,
			CooldownPeriod,
			false
		);

		for (const auto& Itr : CollisionComponents)
		{
			Execute_UnbindCollisionShape(this, Itr);
		}

		OnInteractionCycleCompleted.Broadcast(GetWorld()->GetTimeSeconds(), RemainingLifecycleCount, Execute_GetInteractor(this));
		return true;
	}

	return false;
}

void UActorInteractableComponentBase::ToggleWidgetVisibility_Implementation(const bool IsVisible)
{
	if (GetWidget())
	{
		UpdateInteractionWidget();
	}
}

void UActorInteractableComponentBase::BindCollisionShape_Implementation(UPrimitiveComponent* PrimitiveComponent) const
{
	if (!PrimitiveComponent) return;
	
	PrimitiveComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &UActorInteractableComponentBase::OnInteractableBeginOverlap);
	PrimitiveComponent->OnComponentEndOverlap.AddUniqueDynamic(this, &UActorInteractableComponentBase::OnInteractableStopOverlap);

	FCollisionShapeCache CachedValues;
	CachedValues.bGenerateOverlapEvents = PrimitiveComponent->GetGenerateOverlapEvents();
	CachedValues.CollisionEnabled = PrimitiveComponent->GetCollisionEnabled();
	CachedValues.CollisionResponse = GetCollisionResponseToChannel(CollisionChannel);
	
	CachedCollisionShapesSettings.Add(PrimitiveComponent, CachedValues);
	
	PrimitiveComponent->SetGenerateOverlapEvents(true);
	PrimitiveComponent->SetCollisionResponseToChannel(CollisionChannel, ECollisionResponse::ECR_Overlap);

	switch (PrimitiveComponent->GetCollisionEnabled())
	{
		case ECollisionEnabled::NoCollision:
			PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			break;
		case ECollisionEnabled::QueryOnly:
		case ECollisionEnabled::PhysicsOnly:
		case ECollisionEnabled::QueryAndPhysics:
		default: break;
	}
}

void UActorInteractableComponentBase::UnbindCollisionShape_Implementation(UPrimitiveComponent* PrimitiveComponent) const
{
	if(!PrimitiveComponent) return;
	
	PrimitiveComponent->OnComponentBeginOverlap.RemoveDynamic(this, &UActorInteractableComponentBase::OnInteractableBeginOverlap);
	PrimitiveComponent->OnComponentEndOverlap.RemoveDynamic(this, &UActorInteractableComponentBase::OnInteractableStopOverlap);

	if (CachedCollisionShapesSettings.Find(PrimitiveComponent))
	{
		PrimitiveComponent->SetGenerateOverlapEvents(CachedCollisionShapesSettings[PrimitiveComponent].bGenerateOverlapEvents);
		PrimitiveComponent->SetCollisionEnabled(CachedCollisionShapesSettings[PrimitiveComponent].CollisionEnabled);
		PrimitiveComponent->SetCollisionResponseToChannel(CollisionChannel, CachedCollisionShapesSettings[PrimitiveComponent].CollisionResponse);
	}
	else
	{
		PrimitiveComponent->SetGenerateOverlapEvents(true);
		PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		PrimitiveComponent->SetCollisionResponseToChannel(CollisionChannel, ECollisionResponse::ECR_Overlap);
	}
}

void UActorInteractableComponentBase::BindHighlightableMesh_Implementation(UMeshComponent* MeshComponent) const
{
	if (!MeshComponent) return;
	
	MeshComponent->SetRenderCustomDepth(true);
}

void UActorInteractableComponentBase::UnbindHighlightableMesh_Implementation(UMeshComponent* MeshComponent) const
{
	if (!MeshComponent) return;
	
	MeshComponent->SetRenderCustomDepth(false);
}

void UActorInteractableComponentBase::ToggleDebug_Implementation()
{
	DebugSettings.DebugMode = !DebugSettings.DebugMode;
}

void UActorInteractableComponentBase::AutoSetup()
{
	switch (SetupType)
	{
		case ESetupType::EST_FullAll:
			{
				if (GetOwner() == nullptr) break;

				TArray<UPrimitiveComponent*> OwnerPrimitives;
				GetOwner()->GetComponents(OwnerPrimitives);

				TArray<UMeshComponent*> OwnerMeshes;
				GetOwner()->GetComponents(OwnerMeshes);

				for (const auto& Itr : OwnerPrimitives)
				{
					if (Itr)
					{
						Execute_AddCollisionComponent(this, Itr);
					}
				}

				for (const auto& Itr : OwnerMeshes)
				{
					if (Itr)
					{
						Execute_AddHighlightableComponent(this, Itr);
					}
				}
			}
			break;
		case ESetupType::EST_AllParent:
			{
				// Get all Parent Components
				TArray<USceneComponent*> ParentComponents;
				GetParentComponents(ParentComponents);

				// Iterate over them and assign them properly
				if (ParentComponents.Num() > 0)
				{
					for (const auto& Itr : ParentComponents)
					{
						if (UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(Itr))
						{
							Execute_AddCollisionComponent(this, PrimitiveComp);

							if (UMeshComponent* MeshComp = Cast<UMeshComponent>(PrimitiveComp))
							{
								Execute_AddHighlightableComponent(this, MeshComp);
							}
						}
					}
				}
			}
			break;
		case ESetupType::EST_Quick:
			{
				if (USceneComponent* ParentComponent = GetAttachParent())
				{
					if (UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(ParentComponent))
					{
						Execute_AddCollisionComponent(this, PrimitiveComp);

						if (UMeshComponent* MeshComp = Cast<UMeshComponent>(PrimitiveComp))
						{
							Execute_AddHighlightableComponent(this, MeshComp);
						}
					}
				}
			}
			break;
		default:
			break;
	}
	
	Execute_FindAndAddCollisionShapes(this);
	Execute_FindAndAddHighlightableMeshes(this);
}

void UActorInteractableComponentBase::OnCooldownCompletedCallback()
{
	if (!GetWorld())
	{
		LOG_ERROR(TEXT("[TriggerCooldown] Interactable has no World, cannot request OnCooldownCompletedEvent!"))
		return;
	}
	
	for (const auto& Itr : CollisionComponents)
	{
		Execute_BindCollisionShape(this, Itr);
	}
	
	OnCooldownCompleted.Broadcast();
}

bool UActorInteractableComponentBase::ValidateInteractable() const
{
	if (GetWidgetClass().Get() == nullptr)
	{
		LOG_ERROR(TEXT("[%s] Has null Widget Class! Disabled!"), *GetName())
		return false;
	}
	if (GetWidgetClass() != UActorInteractableWidget::StaticClass())
	{
		if (GetWidgetClass()->ImplementsInterface(UActorInteractionWidget::StaticClass()) == false)
		{
			LOG_ERROR(TEXT("[%s] Has invalid Widget Class! Disabled!"), *GetName())
			return false;
		}
	}

	return true;
}

void UActorInteractableComponentBase::UpdateInteractionWidget()
{
	if (UUserWidget* UserWidget = GetWidget() )
	{
		if (UserWidget->Implements<UActorInteractionWidget>())
		{
			TScriptInterface<IActorInteractionWidget> InteractionWidget = UserWidget;
			InteractionWidget.SetObject(UserWidget);
			InteractionWidget.SetInterface(Cast<IActorInteractionWidget>(UserWidget));

			InteractionWidget->Execute_UpdateWidget(UserWidget, this);
		}
	}
}

void UActorInteractableComponentBase::InteractableDependencyStartedCallback_Implementation(const TScriptInterface<IActorInteractableInterface>& NewMaster)
{
	if (NewMaster.GetObject() == nullptr) return;

	// Force lower weight but never higher than it was before.
	const int32 NewWeight = FMath::Min(InteractionWeight, NewMaster->Execute_GetInteractableWeight(this) - 1);
	Execute_SetInteractableWeight(this, NewWeight);
}

void UActorInteractableComponentBase::InteractableDependencyStoppedCallback_Implementation(const TScriptInterface<IActorInteractableInterface>& FormerMaster)
{
	Execute_SetInteractableWeight(this, CachedInteractionWeight);
}

#if (!UE_BUILD_SHIPPING || WITH_EDITOR)
#if WITH_EDITOR

void UActorInteractableComponentBase::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	const FName PropertyName = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.GetPropertyName() : NAME_None;

	FString interactableName = GetName();
	// Format Name
	{
		if (interactableName.Contains(TEXT("_GEN_VARIABLE")))
		{
			interactableName.ReplaceInline(TEXT("_GEN_VARIABLE"), TEXT(""));
		}
		if(interactableName.EndsWith(TEXT("_C")) && interactableName.StartsWith(TEXT("Default__")))
		{
		
			interactableName.RightChopInline(9);
			interactableName.LeftChopInline(2);
		}
	}
	
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UActorInteractableComponentBase, DefaultInteractableState))
	{
		if
		(
			DefaultInteractableState == EInteractableStateV2::EIS_Active			||
			DefaultInteractableState == EInteractableStateV2::EIS_Completed	||
			DefaultInteractableState == EInteractableStateV2::EIS_Cooldown
		)
		{
			const FText ErrorMessage = FText::FromString
			(
				interactableName.Append(TEXT(": DefaultInteractableState cannot be ")).Append(GetEnumValueAsString("EInteractableStateV2", DefaultInteractableState)).Append(TEXT("!"))
			);
			FEditorHelper::DisplayEditorNotification(ErrorMessage, SNotificationItem::CS_Fail, 5.f, 2.f, TEXT("Icons.Error"));

			DefaultInteractableState = EInteractableStateV2::EIS_Awake;
		}
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UActorInteractableComponentBase, LifecycleCount))
	{
		if (LifecycleMode == EInteractableLifecycle::EIL_Cycled)
		{
			if (LifecycleCount == 0 || LifecycleCount == 1)
			{
				if (DebugSettings.EditorDebugMode)
				{
					const FText ErrorMessage = FText::FromString
					(
						interactableName.Append(TEXT(": Cycled LifecycleCount cannot be: ")).Append(FString::FromInt(LifecycleCount)).Append(TEXT("!"))
					);
				
					FEditorHelper::DisplayEditorNotification(ErrorMessage, SNotificationItem::CS_Fail, 5.f, 2.f, TEXT("Icons.Error"));
				}
				
				LifecycleCount = 2.f;
				RemainingLifecycleCount = LifecycleCount;
			}
		}
	}
	
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UActorInteractableComponentBase, InteractionPeriod))
	{
		if (InteractionPeriod < -1.f)
		{
			const FText ErrorMessage = FText::FromString
			(
				interactableName.Append(TEXT(": InteractionPeriod cannot be less than -1!"))
			);
			FEditorHelper::DisplayEditorNotification(ErrorMessage, SNotificationItem::CS_Fail, 5.f, 2.f, TEXT("Icons.Error"));

			InteractionPeriod = -1.f;
		}

		if (InteractionPeriod > -1.f && InteractionPeriod < 0.1f)
		{
			InteractionPeriod = 0.1f;
		}

		if (FMath::IsNearlyZero(InteractionPeriod, 0.001f))
		{
			InteractionPeriod = 0.1f;
		}
	}
		
	if (PropertyName == TEXT("WidgetClass"))
	{
		if (GetWidgetClass() == nullptr)
		{
			const FText ErrorMessage = FText::FromString
			(
				interactableName.Append(TEXT(": Widget Class is NULL!"))
			);
			FEditorHelper::DisplayEditorNotification(ErrorMessage, SNotificationItem::CS_Fail, 5.f, 2.f, TEXT("Icons.Error"));
		}
		else
		{
			if (!GetWidgetClass()->IsChildOf(UActorInteractableWidget::StaticClass()))
			{
				if (GetWidgetClass()->ImplementsInterface(UActorInteractionWidget::StaticClass()) == false)
				{
					const FText ErrorMessage = FText::FromString
					(
						interactableName.Append(TEXT(": Widget Class must either implement 'ActorInteractionWidget Interface' or inherit from 'ActorInteractableWidget' class!"))
					);
					
					SetWidgetClass(nullptr);
					FEditorHelper::DisplayEditorNotification(ErrorMessage, SNotificationItem::CS_Fail, 5.f, 2.f, TEXT("Icons.Error"));
				}
			}
		}
	}

	if (PropertyName == TEXT("Space"))
	{
		if (GetWidgetSpace() == EWidgetSpace::World)
		{
			SetWorldScale3D(FVector(0.01f));
			SetDrawSize(FVector2D(2000));
			bDrawAtDesiredSize = false;
		}
		else
		{
			SetWorldScale3D(FVector(1.f));
			bDrawAtDesiredSize = false;
		}

		const FText ErrorMessage = FText::FromString
		(
			interactableName.Append(TEXT(": UI Space changed! Component Scale has been updated. Update 'DrawSize' to match new Widget Space!"))
		);
		FEditorHelper::DisplayEditorNotification(ErrorMessage, SNotificationItem::CS_Fail, 5.f, 2.f, TEXT("NotificationList.DefaultMessage"));
	}
}

EDataValidationResult UActorInteractableComponentBase::IsDataValid(TArray<FText>& ValidationErrors)
{
	const auto DefaultValue = Super::IsDataValid(ValidationErrors);
	bool bAnyError = false;

	if (DefaultInteractableState == EInteractableStateV2::EIS_Disabled)
	{
		FString interactableName = GetName();
		{
			if (interactableName.Contains(TEXT("_GEN_VARIABLE")))
			{
				interactableName.ReplaceInline(TEXT("_GEN_VARIABLE"), TEXT(""));
			}
			if(interactableName.EndsWith(TEXT("_C")) && interactableName.StartsWith(TEXT("Default__")))
			{
		
				interactableName.RightChopInline(9);
				interactableName.LeftChopInline(2);
			}
		}
	
		if
		(
			DefaultInteractableState == EInteractableStateV2::EIS_Active ||
			DefaultInteractableState == EInteractableStateV2::EIS_Completed ||
			DefaultInteractableState == EInteractableStateV2::EIS_Cooldown
		)
		{
			const FText ErrorMessage = FText::FromString
			(
				interactableName.Append(TEXT(": DefaultInteractableState cannot be")).Append(GetEnumValueAsString("EInteractableStateV2", DefaultInteractableState)).Append(TEXT("!"))
			);

			DefaultInteractableState = EInteractableStateV2::EIS_Awake;
		
			ValidationErrors.Add(ErrorMessage);
			bAnyError = true;
		}

		if (InteractionPeriod < -1.f)
		{
			const FText ErrorMessage = FText::FromString
			(
				interactableName.Append(TEXT(": DefaultInteractableState cannot be lesser than -1!"))
			);

			InteractionPeriod = -1.f;
		
			ValidationErrors.Add(ErrorMessage);
			bAnyError = true;
		}
	
		if (LifecycleMode == EInteractableLifecycle::EIL_Cycled && (LifecycleCount == 0 || LifecycleCount == 1))
		{
			const FText ErrorMessage = FText::FromString
			(
				interactableName.Append(TEXT(":")).Append(TEXT(" LifecycleCount cannot be %d!"), LifecycleCount)
			);
			
			LifecycleCount = 2.f;
			RemainingLifecycleCount = LifecycleCount;
		
			ValidationErrors.Add(ErrorMessage);
			bAnyError = true;
		}

		if (GetWidgetClass() == nullptr)
		{
			const FText ErrorMessage = FText::FromString
			(
				interactableName.Append(TEXT(": Widget Class is NULL!"))
			);

			ValidationErrors.Add(ErrorMessage);
			bAnyError = true;
		}
		else
		{
			if (!GetWidgetClass()->IsChildOf(UActorInteractableWidget::StaticClass()))
			{
				if (GetWidgetClass()->ImplementsInterface(UActorInteractionWidget::StaticClass()) == false)
				{
					const FText ErrorMessage = FText::FromString
					(
						interactableName.Append(TEXT(" : Widget Class must either implement 'ActorInteractionWidget Interface' or inherit from 'ActorInteractableWidget' class!"))
					);

					SetWidgetClass(nullptr);
					ValidationErrors.Add(ErrorMessage);
					bAnyError = true;
				}
			}
		}
	}
	
	return bAnyError ? EDataValidationResult::Invalid : DefaultValue;
}

/*
bool UActorInteractableComponentBase::Modify(bool bAlwaysMarkDirty)
{
	const bool bResult = Super::Modify(bAlwaysMarkDirty);

	const bool bShouldNotify =
	{
		GetOwner() != nullptr &&
		UActorInteractionFunctionLibrary::IsEditorDebugEnabled() &&
		(
			InteractableData.DataTable == nullptr ||
			GetWidgetClass() == nullptr
		)
	};

	if (bShouldNotify)
	{
		FString interactableName = GetName();
		// Format Name
		{
			if (interactableName.Contains(TEXT("_GEN_VARIABLE")))
			{
				interactableName.ReplaceInline(TEXT("_GEN_VARIABLE"), TEXT(""));
			}
			if (interactableName.Contains(TEXT("SKEL_")))
			{
				interactableName.ReplaceInline(TEXT("SKEL_"), TEXT(""));
			}
			if(interactableName.EndsWith(TEXT("_C")) && interactableName.StartsWith(TEXT("Default__")))
			{
		
				interactableName.RightChopInline(9);
				interactableName.LeftChopInline(2);
			}
		}

		FString ownerName;
		GetOwner()->GetName(ownerName);
		
		const FText ErrorMessage = FText::FromString
		(
			interactableName.Append(" from ").Append(ownerName).Append(TEXT(": Interactable Data or Widget Class are not valid! Use 'SetDefaults' to avoid issues!"))
		);
		FEditorHelper::DisplayEditorNotification(ErrorMessage, SNotificationItem::CS_Fail, 5.f, 2.f, TEXT("NotificationList.DefaultMessage"));

	}
	
	return bResult;
}
*/

#endif

void UActorInteractableComponentBase::DrawDebug()
{
	if (DebugSettings.DebugMode)
	{
		for (const auto& Itr : CollisionComponents)
		{
			Itr->SetHiddenInGame(false);
		}
	}
}


#endif

#pragma endregion

#undef LOCTEXT_NAMESPACE