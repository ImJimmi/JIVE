#pragma once

#include "jive_GuiItem.h"

#include <juce_core/juce_core.h>

//======================================================================================================================
namespace juce
{
    template <>
    struct VariantConverter<jive::GuiItem::Display>
    {
        static jive::GuiItem::Display fromVar(const var& v)
        {
            return static_cast<jive::GuiItem::Display>(static_cast<int>(v));
        }

        static var toVar (jive::GuiItem::Display display)
        {
            return static_cast<int>(display);
        }
    };
} // namespace juce
