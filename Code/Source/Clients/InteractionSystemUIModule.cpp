
#include <InteractionSystemUI/InteractionSystemUITypeIds.h>
#include <InteractionSystemUIModuleInterface.h>
#include "InteractionSystemUISystemComponent.h"

namespace InteractionSystemUI
{
    class InteractionSystemUIModule
        : public InteractionSystemUIModuleInterface
    {
    public:
        AZ_RTTI(InteractionSystemUIModule, InteractionSystemUIModuleTypeId, InteractionSystemUIModuleInterface);
        AZ_CLASS_ALLOCATOR(InteractionSystemUIModule, AZ::SystemAllocator);
    };
}// namespace InteractionSystemUI

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), InteractionSystemUI::InteractionSystemUIModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_InteractionSystemUI, InteractionSystemUI::InteractionSystemUIModule)
#endif
