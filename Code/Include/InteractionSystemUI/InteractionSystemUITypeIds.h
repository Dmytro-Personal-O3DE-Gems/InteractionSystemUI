
#pragma once

namespace InteractionSystemUI
{
    // System Component TypeIds
    inline constexpr const char* InteractionSystemUISystemComponentTypeId = "{68307F12-1A87-4401-8711-CCB6D02C8301}";
    inline constexpr const char* InteractionSystemUIEditorSystemComponentTypeId = "{0068E7BA-80B8-4310-BB51-6297E49A2229}";

    // Module derived classes TypeIds
    inline constexpr const char* InteractionSystemUIModuleInterfaceTypeId = "{6DB872A9-99A6-4665-85C8-7E92A43E7F31}";
    inline constexpr const char* InteractionSystemUIModuleTypeId = "{AD4501FB-5664-40FD-B969-6AC83939CDDD}";
    // The Editor Module by default is mutually exclusive with the Client Module
    // so they use the Same TypeId
    inline constexpr const char* InteractionSystemUIEditorModuleTypeId = InteractionSystemUIModuleTypeId;

    // Interface TypeIds
    inline constexpr const char* InteractionSystemUIRequestsTypeId = "{57B42C92-FACE-48A0-A691-9C6BBCBB3CB5}";
} // namespace InteractionSystemUI
