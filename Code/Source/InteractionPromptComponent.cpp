#include "InteractionPromptComponent.h"

#include <AzCore/Component/Entity.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

#include <InteractionSystem/InteractableInterface.h>   // what the target supports (InteractionInfo)

#include <LyShine/Bus/World/UiCanvasRefBus.h>   // canvas id from "UI Canvas Asset Ref"
#include <LyShine/Bus/UiCanvasBus.h>            // find element by name
#include <LyShine/Bus/UiElementBus.h>           // show / hide
#include <LyShine/Bus/UiTextBus.h>              // label text
#include <LyShine/Bus/UiImageBus.h>             // ring fill amount

#include <cmath>                                // std::exp for the ring decay

namespace InteractionSystemUI
{
    namespace
    {
        //! Text to put on screen for a label.
        //! TODO: translate the key through LocalizationContract first; use this fallback only when there is no translation.
        //! Fallback order (see InteractionSystem::LocalizedText): the default text, and if that is empty, the key itself.
        AZStd::string GetDisplayText(const InteractionSystem::LocalizedText& label)
        {
            return label.m_defaultText.empty() ? label.m_key : label.m_defaultText;
        }
    } // namespace

    AZ_COMPONENT_IMPL(InteractionPromptComponent, "InteractionPromptComponent", "{CDB5EDAC-2546-487A-86CB-DE1322C4AFFE}");

    void InteractionPromptComponent::Activate()
    {
        // "UI Canvas Asset Ref" is a required service, so it is already active here. With "Load automatically"
        // on, it has already loaded the canvas inside its own Activate(), so the id is available right away.
        // The canvas id itself is not kept: it is only needed to find the elements.
        AZ::EntityId canvasId;
        UiCanvasRefBus::EventResult(canvasId, GetEntityId(), &UiCanvasRefBus::Events::GetCanvas);
        if (!canvasId.IsValid())
        {
            AZ_Warning("InteractionPrompt", false,
                "Entity '%s' has no loaded UI canvas. Enable 'Load automatically' on its UI Canvas Asset Ref.",
                GetEntity()->GetName().c_str());
            return; // nothing to show, so do not listen to the finder either
        }

        // Element ids are new on every canvas load, so they are looked up here and never serialized.
        m_pressRowId = FindElement(canvasId, m_pressRowName);
        m_holdRowId = FindElement(canvasId, m_holdRowName);
        m_pressOrHoldRowId = FindElement(canvasId, m_pressOrHoldRowName);

        // Labels are looked up inside their rows, so the same name may be reused in different rows.
        m_pressRowLabelId = FindChildElement(m_pressRowId, m_pressRowLabelName);
        m_holdRowLabelId = FindChildElement(m_holdRowId, m_holdRowLabelName);
        m_pressOrHoldPressLabelId = FindChildElement(m_pressOrHoldRowId, m_pressOrHoldPressLabelName);
        m_pressOrHoldHoldLabelId = FindChildElement(m_pressOrHoldRowId, m_pressOrHoldHoldLabelName);

        // One ring name for both rows: each lookup is limited to its own row.
        m_holdRowRingFillId = FindChildElement(m_holdRowId, m_ringFillName);
        m_pressOrHoldRingFillId = FindChildElement(m_pressOrHoldRowId, m_ringFillName);

        // Pull: the finder may already have a target (e.g. the player spawned facing a door).
        // OnTargetChanged is sent only on a change, so without this the prompt would stay empty until the target changes.
        // If the finder is not active yet, the id stays invalid and all rows are hidden (the start state).
        AZ::EntityId currentTargetId;
        InteractionSystem::InteractionOverlapFinderRequestBus::EventResult(
            currentTargetId, GetEntityId(), &InteractionSystem::InteractionOverlapFinderRequests::GetCurrentTarget);
        UpdatePrompt(currentTargetId);

        // Push: every later change. The finder sends to its owner entity, which is this entity.
        InteractionSystem::InteractionOverlapFinderNotificationBus::Handler::BusConnect(GetEntityId());

        // The interactor sits on this entity (the player): the same address the finder reports to.
        InteractionSystem::InteractorNotificationBus::Handler::BusConnect(GetEntityId());
    }

    void InteractionPromptComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        InteractionSystem::InteractionOverlapFinderNotificationBus::Handler::BusDisconnect();
        InteractionSystem::InteractorNotificationBus::Handler::BusDisconnect();

        // The canvas is released when the UI Canvas Asset Ref deactivates (right after this component),
        // so these ids are about to become dead anyway.
        m_pressRowId.SetInvalid();
        m_holdRowId.SetInvalid();
        m_pressOrHoldRowId.SetInvalid();
        m_pressRowLabelId.SetInvalid();
        m_holdRowLabelId.SetInvalid();
        m_pressOrHoldPressLabelId.SetInvalid();
        m_pressOrHoldHoldLabelId.SetInvalid();
        m_holdRowRingFillId.SetInvalid();
        m_pressOrHoldRingFillId.SetInvalid();

        m_targetId.SetInvalid();
        m_shownPressLabel.clear();
        m_shownHoldLabel.clear();
        m_shownRingFill = 0.0f;
    }

    void InteractionPromptComponent::OnTargetChanged([[maybe_unused]] AZ::EntityId previousTargetId, AZ::EntityId newTargetId)
    {
        UpdatePrompt(newTargetId);
    }

    void InteractionPromptComponent::OnInteractionDenied(
        [[maybe_unused]] AZ::EntityId targetId, [[maybe_unused]] InteractionSystem::InteractionType type)
    {
        ResetRing();
    }

    void InteractionPromptComponent::OnInteractionDone(
        [[maybe_unused]] AZ::EntityId targetId, [[maybe_unused]] InteractionSystem::InteractionType type)
    {
        ResetRing();
    }

    void InteractionPromptComponent::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        // Polling instead of events: a label can change for reasons the target does not know about
        // (e.g. the player picked up a key), so asking every frame catches all of them.
        RefreshLabels(false);

        // The progress changes every frame while a hold runs, so it is polled too.
        RefreshRing(deltaTime);
    }

    AZ::EntityId InteractionPromptComponent::FindElement(AZ::EntityId canvasId, const AZStd::string& elementName) const
    {
        AZ::EntityId elementId;
        UiCanvasBus::EventResult(elementId, canvasId, &UiCanvasBus::Events::FindElementEntityIdByName, elementName);
        AZ_Warning("InteractionPrompt", elementId.IsValid(),
            "UI element '%s' was not found on the canvas. Check the name in the Interaction Prompt component.",
            elementName.c_str());
        return elementId;
    }

    AZ::EntityId InteractionPromptComponent::FindChildElement(AZ::EntityId parentId, const AZStd::string& elementName) const
    {
        // Same search as on the canvas (all descendants, first match), but starting from the given element.
        AZ::EntityId elementId;
        UiElementBus::EventResult(elementId, parentId, &UiElementBus::Events::FindDescendantEntityIdByName, elementName);
        AZ_Warning("InteractionPrompt", !parentId.IsValid() || elementId.IsValid(),
            "UI element '%s' was not found inside its row. Check the name in the Interaction Prompt component.",
            elementName.c_str());
        return elementId;
    }

    void InteractionPromptComponent::UpdatePrompt(AZ::EntityId targetId)
    {
        bool showPress = false;
        bool showHold = false;
        bool showPressOrHold = false;

        // The finder only reports entities with an Interactable, but the target may have been deactivated since.
        // Without this check EventResult would leave the default InteractionInfo (Press) and show a wrong prompt.
        const bool hasTarget = targetId.IsValid() && InteractionSystem::InteractableRequestBus::HasHandlers(targetId);
        if (hasTarget)
        {
            InteractionSystem::InteractionInfo info;
            InteractionSystem::InteractableRequestBus::EventResult(
                info, targetId, &InteractionSystem::InteractableRequests::GetInteractionInfo);

            switch (info.m_mode)
            {
            case InteractionSystem::InteractionMode::Press:
                showPress = true;
                break;
            case InteractionSystem::InteractionMode::Hold:
                showHold = true;
                break;
            case InteractionSystem::InteractionMode::PressOrHold:
                showPressOrHold = true;
                break;
            }
        }

        if (hasTarget)
        {
            m_targetId = targetId;
            // What is on screen belongs to the previous target: write the new labels unconditionally,
            // even if the text happens to be the same. Done before showing the row, so no stale frame is visible.
            RefreshLabels(true);
            AZ::TickBus::Handler::BusConnect(); // connecting twice is a no-op
        }
        else
        {
            m_targetId.SetInvalid();
            AZ::TickBus::Handler::BusDisconnect();
        }

        // An invalid element id (not found) has no handler, so these calls simply do nothing.
        UiElementBus::Event(m_pressRowId, &UiElementBus::Events::SetIsEnabled, showPress);
        UiElementBus::Event(m_holdRowId, &UiElementBus::Events::SetIsEnabled, showHold);
        UiElementBus::Event(m_pressOrHoldRowId, &UiElementBus::Events::SetIsEnabled, showPressOrHold);

        // The progress belonged to the previous target (the interactor cancels that attempt anyway).
        ResetRing();
    }

    void InteractionPromptComponent::RefreshLabels(bool force)
    {
        // The target may have been deactivated without the finder noticing yet: keep the last labels then.
        if (!InteractionSystem::InteractableRequestBus::HasHandlers(m_targetId))
        {
            return;
        }

        InteractionSystem::LocalizedText pressLabel;
        InteractionSystem::InteractableRequestBus::EventResult(
            pressLabel, m_targetId, &InteractionSystem::InteractableRequests::GetInteractableLabel,
            GetEntityId(), InteractionSystem::InteractionType::Press);
        const AZStd::string pressText = GetDisplayText(pressLabel);
        if (force || pressText != m_shownPressLabel)
        {
            SetLabelText(m_pressRowLabelId, m_pressOrHoldPressLabelId, pressText);
            m_shownPressLabel = pressText;
        }

        InteractionSystem::LocalizedText holdLabel;
        InteractionSystem::InteractableRequestBus::EventResult(
            holdLabel, m_targetId, &InteractionSystem::InteractableRequests::GetInteractableLabel,
            GetEntityId(), InteractionSystem::InteractionType::Hold);
        const AZStd::string holdText = GetDisplayText(holdLabel);
        if (force || holdText != m_shownHoldLabel)
        {
            SetLabelText(m_holdRowLabelId, m_pressOrHoldHoldLabelId, holdText);
            m_shownHoldLabel = holdText;
        }
    }

    void InteractionPromptComponent::SetLabelText(
        AZ::EntityId singleRowLabelId, AZ::EntityId pressOrHoldLabelId, const AZStd::string& label)
    {
        // Both rows get the text, even the hidden one: then switching rows never shows an outdated label.
        UiTextBus::Event(singleRowLabelId, &UiTextBus::Events::SetText, label);
        UiTextBus::Event(pressOrHoldLabelId, &UiTextBus::Events::SetText, label);
    }

    void InteractionPromptComponent::RefreshRing(float deltaTime)
    {
        // No interactor on this entity: EventResult leaves 0, so the ring just stays empty.
        float realProgress = 0.0f;
        InteractionSystem::InteractorRequestBus::EventResult(
            realProgress, GetEntityId(), &InteractionSystem::InteractorRequests::GetHoldProgress);

        float shown = m_shownRingFill;
        if (realProgress >= shown || m_ringDecaySpeed <= 0.0f)
        {
            // Growing: follow the real value exactly, any lag would make the hold look slower than it is.
            shown = realProgress;
        }
        else
        {
            // Released early: exponential decay. The same share of the remaining distance goes every second,
            // independent of the frame rate (unlike "shown -= step", which would depend on fps).
            shown += (realProgress - shown) * (1.0f - std::exp(-m_ringDecaySpeed * deltaTime));

            // The exponent never reaches the target exactly; snap the last invisible bit.
            if (shown - realProgress < 0.001f)
            {
                shown = realProgress;
            }
        }

        // Most frames nothing changes (no hold running): skip the UI call then.
        if (shown != m_shownRingFill)
        {
            SetRingFill(shown);
        }
    }

    void InteractionPromptComponent::ResetRing()
    {
        SetRingFill(0.0f);
    }

    void InteractionPromptComponent::SetRingFill(float fillAmount)
    {
        m_shownRingFill = fillAmount;
        // Both rings get the value, even the hidden one, same as the labels.
        UiImageBus::Event(m_holdRowRingFillId, &UiImageBus::Events::SetFillAmount, fillAmount);
        UiImageBus::Event(m_pressOrHoldRingFillId, &UiImageBus::Events::SetFillAmount, fillAmount);
    }

    void InteractionPromptComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<InteractionPromptComponent, AZ::Component>()
                ->Version(1)
                ->Field("PressRowName", &InteractionPromptComponent::m_pressRowName)
                ->Field("HoldRowName", &InteractionPromptComponent::m_holdRowName)
                ->Field("PressOrHoldRowName", &InteractionPromptComponent::m_pressOrHoldRowName)
                ->Field("PressRowLabelName", &InteractionPromptComponent::m_pressRowLabelName)
                ->Field("HoldRowLabelName", &InteractionPromptComponent::m_holdRowLabelName)
                ->Field("PressOrHoldPressLabelName", &InteractionPromptComponent::m_pressOrHoldPressLabelName)
                ->Field("PressOrHoldHoldLabelName", &InteractionPromptComponent::m_pressOrHoldHoldLabelName)
                ->Field("RingFillName", &InteractionPromptComponent::m_ringFillName)
                ->Field("RingDecaySpeed", &InteractionPromptComponent::m_ringDecaySpeed)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<InteractionPromptComponent>("Interaction Prompt",
                    "Shows the UI canvas element that matches the interaction mode of the interactable the owner is looking at.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Interaction")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->DataElement(AZ::Edit::UIHandlers::Default, &InteractionPromptComponent::m_pressRowName,
                        "Press Row", "Name of the canvas element shown when the target supports only a press.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &InteractionPromptComponent::m_holdRowName,
                        "Hold Row", "Name of the canvas element shown when the target supports only a hold.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &InteractionPromptComponent::m_pressOrHoldRowName,
                        "Press Or Hold Row", "Name of the canvas element shown when the target supports both a press and a hold.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &InteractionPromptComponent::m_pressRowLabelName,
                        "Press Row Label", "Name of the label text inside the Press Row. Searched only inside that row.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &InteractionPromptComponent::m_holdRowLabelName,
                        "Hold Row Label", "Name of the label text inside the Hold Row. Searched only inside that row.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &InteractionPromptComponent::m_pressOrHoldPressLabelName,
                        "Press Or Hold: Press Label", "Name of the press label text inside the Press Or Hold Row.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &InteractionPromptComponent::m_pressOrHoldHoldLabelName,
                        "Press Or Hold: Hold Label", "Name of the hold label text inside the Press Or Hold Row.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &InteractionPromptComponent::m_ringFillName,
                        "Ring Fill", "Name of the hold progress image (Fill Type: Radial). Searched inside the Hold Row and inside the Press Or Hold Row.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &InteractionPromptComponent::m_ringDecaySpeed,
                        "Ring Decay Speed", "How fast the ring drains after a hold is released early, in 1/s. "
                        "Part left after t seconds = exp(-speed * t): 10 leaves 5% after 0.3 s. 0 = empty at once.")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                    ;
            }
        }

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<InteractionPromptComponent>("InteractionPrompt Component Group")
                ->Attribute(AZ::Script::Attributes::Category, "Interaction")
                ;
        }
    }

    void InteractionPromptComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("InteractionPromptComponentService"));
    }

    void InteractionPromptComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("InteractionPromptComponentService"));
    }

    void InteractionPromptComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        // The canvas comes from "UI Canvas Asset Ref" on the same entity; this also makes it activate first.
        // The target finder is NOT listed: it sits on the camera (another entity) and services work per entity only.
        required.push_back(AZ_CRC_CE("UiCanvasRefService"));
    }

    void InteractionPromptComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }
} // namespace InteractionSystemUI
