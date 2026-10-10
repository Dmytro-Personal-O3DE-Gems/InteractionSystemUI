
#include "InteractionSystemUISystemComponent.h"

#include <InteractionSystemUI/InteractionSystemUITypeIds.h>

#include <AzCore/Serialization/SerializeContext.h>

namespace InteractionSystemUI
{
    AZ_COMPONENT_IMPL(InteractionSystemUISystemComponent, "InteractionSystemUISystemComponent",
        InteractionSystemUISystemComponentTypeId);

    void InteractionSystemUISystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<InteractionSystemUISystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void InteractionSystemUISystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("InteractionSystemUIService"));
    }

    void InteractionSystemUISystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("InteractionSystemUIService"));
    }

    void InteractionSystemUISystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void InteractionSystemUISystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    InteractionSystemUISystemComponent::InteractionSystemUISystemComponent()
    {
        if (InteractionSystemUIInterface::Get() == nullptr)
        {
            InteractionSystemUIInterface::Register(this);
        }
    }

    InteractionSystemUISystemComponent::~InteractionSystemUISystemComponent()
    {
        if (InteractionSystemUIInterface::Get() == this)
        {
            InteractionSystemUIInterface::Unregister(this);
        }
    }

    void InteractionSystemUISystemComponent::Init()
    {
    }

    void InteractionSystemUISystemComponent::Activate()
    {
        InteractionSystemUIRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void InteractionSystemUISystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        InteractionSystemUIRequestBus::Handler::BusDisconnect();
    }

    void InteractionSystemUISystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace InteractionSystemUI
