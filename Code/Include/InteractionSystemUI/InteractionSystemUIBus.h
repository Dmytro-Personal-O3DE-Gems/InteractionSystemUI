
#pragma once

#include <InteractionSystemUI/InteractionSystemUITypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace InteractionSystemUI
{
    class InteractionSystemUIRequests
    {
    public:
        AZ_RTTI(InteractionSystemUIRequests, InteractionSystemUIRequestsTypeId);
        virtual ~InteractionSystemUIRequests() = default;
        // Put your public methods here
    };

    class InteractionSystemUIBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using InteractionSystemUIRequestBus = AZ::EBus<InteractionSystemUIRequests, InteractionSystemUIBusTraits>;
    using InteractionSystemUIInterface = AZ::Interface<InteractionSystemUIRequests>;

} // namespace InteractionSystemUI
