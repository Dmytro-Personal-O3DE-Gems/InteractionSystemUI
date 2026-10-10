
#include "InteractionSystemUIModuleInterface.h"
#include <AzCore/Memory/Memory.h>

#include <InteractionSystemUI/InteractionSystemUITypeIds.h>

#include <Clients/InteractionSystemUISystemComponent.h>
#include "InteractionPromptComponent.h"

namespace InteractionSystemUI
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(InteractionSystemUIModuleInterface,
        "InteractionSystemUIModuleInterface", InteractionSystemUIModuleInterfaceTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(InteractionSystemUIModuleInterface, AZ::Module);
    AZ_CLASS_ALLOCATOR_IMPL(InteractionSystemUIModuleInterface, AZ::SystemAllocator);

    InteractionSystemUIModuleInterface::InteractionSystemUIModuleInterface()
    {
        // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
        // Add ALL components descriptors associated with this gem to m_descriptors.
        // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
        // This happens through the [MyComponent]::Reflect() function.
        m_descriptors.insert(m_descriptors.end(), {
            InteractionSystemUISystemComponent::CreateDescriptor(),
            InteractionPromptComponent::CreateDescriptor(),
            });
    }

    AZ::ComponentTypeList InteractionSystemUIModuleInterface::GetRequiredSystemComponents() const
    {
        return AZ::ComponentTypeList{
            azrtti_typeid<InteractionSystemUISystemComponent>(),
        };
    }
} // namespace InteractionSystemUI
