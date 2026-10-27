
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/string/string.h>
#include <InteractionSystem/InteractionOverlapFinderInterface.h>
#include <InteractionSystem/InteractorInterface.h>
#include <InteractionSystem/InteractorInputInterface.h> // InteractActionName

namespace InteractionSystemUI
{
    //! Shows the interaction prompt for the interactable the owner is looking at.
    //! The canvas has one element per interaction mode; exactly one of them is shown, or none without a target.
    //! (One element per mode instead of hiding parts of a Layout Row: a Layout Row keeps space for disabled children.)
    //! Each row has a static key hint and label texts. The labels come from the target
    //! (InteractableRequests::GetInteractableLabel) and are polled every frame while there is a target,
    //! so any change (door opened, key picked up) shows up without extra events.
    //! Labels are translated through LocalizationContract when an implementation is active,
    //! otherwise their default texts are shown (see InteractionSystem::LocalizedText).
    //! The key hint is asked from InputHintsContract and polled together with the labels.
    //! Without an implementation the key hint text set in the canvas stays as is.
    class InteractionPromptComponent
        : public AZ::Component
        , public InteractionSystem::InteractionOverlapFinderNotificationBus::Handler
        , public InteractionSystem::InteractorNotificationBus::Handler
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(InteractionPromptComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        // AZ::Component
        void Activate() override;
        void Deactivate() override;

        // InteractionOverlapFinderNotificationBus
        void OnTargetChanged(AZ::EntityId previousTargetId, AZ::EntityId newTargetId) override;

        // InteractorNotificationBus: an attempt finished, so the ring is emptied at once (no smooth decay)
        void OnInteractionDenied(AZ::EntityId targetId, InteractionSystem::InteractionType type) override;
        void OnInteractionDone(AZ::EntityId targetId, InteractionSystem::InteractionType type) override;

        // AZ::TickBus: connected only while there is a target
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

    private:
        //! Finds a canvas element by name. Returns an invalid id (and warns) if there is no such element.
        AZ::EntityId FindElement(AZ::EntityId canvasId, const AZStd::string& elementName) const;

        //! Finds an element by name among the descendants of parentId (a row), not in the whole canvas.
        //! Returns an invalid id (and warns) if there is no such element.
        AZ::EntityId FindChildElement(AZ::EntityId parentId, const AZStd::string& elementName) const;

        //! Shows the row that matches the target's interaction mode, hides everything for an invalid target,
        //! remembers the target and starts or stops polling its labels.
        void UpdatePrompt(AZ::EntityId targetId);

        //! Asks the current target for its labels and writes the ones that changed to the canvas.
        //! force: write even if the text equals the last shown one (used right after the target changed).
        void RefreshLabels(bool force);

        //! Asks InputHintsContract which key is bound to the interact action and writes it to the key hint elements
        //! if it changed. No implementation: the canvas text stays. An implementation without a binding: m_unboundKeyText.
        void RefreshKeyHint();

        //! Writes one label to the text elements that show it (the single-mode row and the press-or-hold row).
        void SetLabelText(AZ::EntityId singleRowLabelId, AZ::EntityId pressOrHoldLabelId, const AZStd::string& label);

        //! Reads the hold progress from the interactor and moves the shown value towards it:
        //! up at once, down smoothly (a hold released early drains instead of blinking out).
        void RefreshRing(float deltaTime);

        //! Empties the ring right away. For a finished attempt or a new target: the old progress is meaningless there.
        void ResetRing();

        //! Writes the fill amount to both ring images (the hold row and the press-or-hold row).
        void SetRingFill(float fillAmount);

        // Element names as they appear in the UI Editor hierarchy. Set in the inspector.
        // Row names must be unique on the canvas: the lookup returns the first match in the whole hierarchy.
        AZStd::string m_pressRowName = "PressRow";
        AZStd::string m_holdRowName = "HoldRow";
        AZStd::string m_pressOrHoldRowName = "PressOrHoldRow";

        // Label names are looked up inside their row only, so they just have to be unique within the row.
        AZStd::string m_pressRowLabelName = "PressLabel";
        AZStd::string m_holdRowLabelName = "HoldLabel";
        AZStd::string m_pressOrHoldPressLabelName = "OrPressLabel";
        AZStd::string m_pressOrHoldHoldLabelName = "OnHoldLabel";

        // Key hint names, looked up inside their rows like the labels (the same name may be reused in different rows).
        AZStd::string m_pressRowKeyHintName = "PressKeyHint";
        AZStd::string m_holdRowKeyHintName = "HoldKeyHint";
        AZStd::string m_pressOrHoldPressKeyHintName = "OrKeyHint";
        AZStd::string m_pressOrHoldHoldKeyHintName = "HoldKeyHint";

        // The input action whose key is shown. Defaults to the action InteractorInputComponent listens to.
        AZStd::string m_interactActionName = InteractionSystem::InteractActionName;

        // Shown when the input hints implementation reports no key for the action (the action is not bound).
        AZStd::string m_unboundKeyText = "-";

        // The ring fill image is looked up inside the Hold Row and inside the Press Or Hold Row, by the same name.
        AZStd::string m_ringFillName = "RingFill";

        // How fast the ring drains after a hold is released early, in 1/s.
        // Part left after t seconds = exp(-speed * t): with 10, 37% after 0.1 s and 5% after 0.3 s. 0 = drop at once.
        float m_ringDecaySpeed = 10.0f;

        // Runtime only, not serialized: elements get new ids every time the canvas is loaded.
        AZ::EntityId m_pressRowId;
        AZ::EntityId m_holdRowId;
        AZ::EntityId m_pressOrHoldRowId;
        AZ::EntityId m_pressRowLabelId;
        AZ::EntityId m_holdRowLabelId;
        AZ::EntityId m_pressOrHoldPressLabelId;
        AZ::EntityId m_pressOrHoldHoldLabelId;
        AZ::EntityId m_pressRowKeyHintId;
        AZ::EntityId m_holdRowKeyHintId;
        AZ::EntityId m_pressOrHoldPressKeyHintId;
        AZ::EntityId m_pressOrHoldHoldKeyHintId;
        AZ::EntityId m_holdRowRingFillId;
        AZ::EntityId m_pressOrHoldRingFillId;

        // State that must survive between frames:
        AZ::EntityId m_targetId;              //!< whom to ask for labels in OnTick
        AZStd::string m_shownPressLabel;      //!< what is on screen now, to call SetText only on change
        AZStd::string m_shownHoldLabel;
        AZStd::string m_shownKeyHint;         //!< empty = nothing written yet, the canvas text is on screen
        float m_shownRingFill = 0.0f;         //!< the smoothed value on screen; the decay continues from it next frame
    };
} // namespace InteractionSystemUI
