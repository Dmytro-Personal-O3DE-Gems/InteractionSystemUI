
#include <InteractionSystemUI/InteractionSystemUITypeIds.h>
#include <InteractionSystemUIModuleInterface.h>
#include "InteractionSystemUIEditorSystemComponent.h"

namespace InteractionSystemUI
{
    class InteractionSystemUIEditorModule
        : public InteractionSystemUIModuleInterface
    {
    public:
        AZ_RTTI(InteractionSystemUIEditorModule, InteractionSystemUIEditorModuleTypeId, InteractionSystemUIModuleInterface);
        AZ_CLASS_ALLOCATOR(InteractionSystemUIEditorModule, AZ::SystemAllocator);

        InteractionSystemUIEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                InteractionSystemUIEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<InteractionSystemUIEditorSystemComponent>(),
            };
        }
    };
}// namespace InteractionSystemUI

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), InteractionSystemUI::InteractionSystemUIEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_InteractionSystemUI_Editor, InteractionSystemUI::InteractionSystemUIEditorModule)
#endif
