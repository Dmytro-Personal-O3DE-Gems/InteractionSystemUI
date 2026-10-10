
#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Clients/InteractionSystemUISystemComponent.h>

namespace InteractionSystemUI
{
    /// System component for InteractionSystemUI editor
    class InteractionSystemUIEditorSystemComponent
        : public InteractionSystemUISystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = InteractionSystemUISystemComponent;
    public:
        AZ_COMPONENT_DECL(InteractionSystemUIEditorSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        InteractionSystemUIEditorSystemComponent();
        ~InteractionSystemUIEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;
    };
} // namespace InteractionSystemUI
