
#include <AzCore/Serialization/SerializeContext.h>
#include "InteractionSystemUIEditorSystemComponent.h"

#include <InteractionSystemUI/InteractionSystemUITypeIds.h>

namespace InteractionSystemUI
{
    AZ_COMPONENT_IMPL(InteractionSystemUIEditorSystemComponent, "InteractionSystemUIEditorSystemComponent",
        InteractionSystemUIEditorSystemComponentTypeId, BaseSystemComponent);

    void InteractionSystemUIEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<InteractionSystemUIEditorSystemComponent, InteractionSystemUISystemComponent>()
                ->Version(0);
        }
    }

    InteractionSystemUIEditorSystemComponent::InteractionSystemUIEditorSystemComponent() = default;

    InteractionSystemUIEditorSystemComponent::~InteractionSystemUIEditorSystemComponent() = default;

    void InteractionSystemUIEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("InteractionSystemUIEditorService"));
    }

    void InteractionSystemUIEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("InteractionSystemUIEditorService"));
    }

    void InteractionSystemUIEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void InteractionSystemUIEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void InteractionSystemUIEditorSystemComponent::Activate()
    {
        InteractionSystemUISystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void InteractionSystemUIEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        InteractionSystemUISystemComponent::Deactivate();
    }

} // namespace InteractionSystemUI
