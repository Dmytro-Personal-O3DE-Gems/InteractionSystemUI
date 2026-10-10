
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/string/string.h>
#include <InteractionSystem/InteractionOverlapFinderInterface.h>

namespace InteractionSystemUI
{
    //! Shows the interaction prompt for the interactable the owner is looking at.
    //! The canvas has one element per interaction mode; exactly one of them is shown, or none without a target.
    //! (One element per mode instead of hiding parts of a Layout Row: a Layout Row keeps space for disabled children.)
    //! Each row has a static key hint and label texts. The labels come from the target
    //! (InteractableRequests::GetInteractableLabel) and are polled every frame while there is a target,
    //! so any change (door opened, key picked up) shows up without extra events.
    class InteractionPromptComponent
        : public AZ::Component
        , public InteractionSystem::InteractionOverlapFinderNotificationBus::Handler
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

        //! Writes one label to the text elements that show it (the single-mode row and the press-or-hold row).
        void SetLabelText(AZ::EntityId singleRowLabelId, AZ::EntityId pressOrHoldLabelId, const AZStd::string& label);

        // Element names as they appear in the UI Editor hierarchy. Set in the inspector.
        // Row names must be unique on the canvas: the lookup returns the first match in the whole hierarchy.
        AZStd::string m_pressRowName = "PressRow";
        AZStd::string m_holdRowName = "HoldRow";
        AZStd::string m_pressOrHoldRowName = "PressOrHoldRow";

        // Label names are looked up inside their row only, so they just have to be unique within the row.
        AZStd::string m_pressRowLabelName = "PressLabel";
        AZStd::string m_holdRowLabelName = "HoldLabel";
        AZStd::string m_pressOrHoldPressLabelName = "OrPressLabel";
        AZStd::string m_pressOrHoldHoldLabelName = "OrHoldLabel";

        // Runtime only, not serialized: elements get new ids every time the canvas is loaded.
        AZ::EntityId m_pressRowId;
        AZ::EntityId m_holdRowId;
        AZ::EntityId m_pressOrHoldRowId;
        AZ::EntityId m_pressRowLabelId;
        AZ::EntityId m_holdRowLabelId;
        AZ::EntityId m_pressOrHoldPressLabelId;
        AZ::EntityId m_pressOrHoldHoldLabelId;

        // State that must survive between frames:
        AZ::EntityId m_targetId;              //!< whom to ask for labels in OnTick
        AZStd::string m_shownPressLabel;      //!< what is on screen now, to call SetText only on change
        AZStd::string m_shownHoldLabel;
    };
} // namespace InteractionSystemUI
