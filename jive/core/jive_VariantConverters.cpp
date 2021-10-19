#include "jive_VariantConverters.h"

//======================================================================================================================
namespace juce
{
    //==================================================================================================================
    const Array<var> VariantConverter<jive::GuiItem::Display>::varArray = {
        "flex",
        "grid"
    };

    jive::GuiItem::Display VariantConverter<jive::GuiItem::Display>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<jive::GuiItem::Display>(varArray.indexOf(v));
    }

    var VariantConverter<jive::GuiItem::Display>::toVar(jive::GuiItem::Display display)
    {
        const auto index = static_cast<int>(display);

        jassert(varArray.size() >= index);
        return varArray[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<juce::FlexBox::Direction>::varArray = {
        "row",
        "row-reverse",
        "column",
        "column-reverse"
    };

    juce::FlexBox::Direction VariantConverter<juce::FlexBox::Direction>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<juce::FlexBox::Direction>(varArray.indexOf(v));
    }

    var VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction direction)
    {
        const auto index = static_cast<int>(direction);

        jassert(varArray.size() >= index);
        return varArray[index];
    }
} // namespace juce
