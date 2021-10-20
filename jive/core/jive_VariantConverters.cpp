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

    //==================================================================================================================
    const Array<var> VariantConverter<juce::FlexBox::Wrap>::varArray = {
        "nowrap",
        "wrap",
        "wrap-reverse"
    };

    juce::FlexBox::Wrap VariantConverter<juce::FlexBox::Wrap>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<juce::FlexBox::Wrap>(varArray.indexOf(v));
    }

    var VariantConverter<juce::FlexBox::Wrap>::toVar(juce::FlexBox::Wrap wrap)
    {
        const auto index = static_cast<int>(wrap);

        jassert(varArray.size() >= index);
        return varArray[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<juce::FlexBox::JustifyContent>::varArray = {
        "flex-start",
        "flex-end",
        "centre",
        "space-between",
        "space-around"
    };

    juce::FlexBox::JustifyContent VariantConverter<juce::FlexBox::JustifyContent>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<juce::FlexBox::JustifyContent>(varArray.indexOf(v));
    }

    var VariantConverter<juce::FlexBox::JustifyContent>::toVar(juce::FlexBox::JustifyContent justification)
    {
        const auto index = static_cast<int>(justification);

        jassert(varArray.size() >= index);
        return varArray[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<juce::FlexBox::AlignContent>::varArray = {
        "stretch",
        "flex-start",
        "flex-end",
        "centre",
        "space-between",
        "space-around"
    };

    juce::FlexBox::AlignContent VariantConverter<juce::FlexBox::AlignContent>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<juce::FlexBox::AlignContent>(varArray.indexOf(v));
    }

    var VariantConverter<juce::FlexBox::AlignContent>::toVar(juce::FlexBox::AlignContent justification)
    {
        const auto index = static_cast<int>(justification);

        jassert(varArray.size() >= index);
        return varArray[index];
    }
} // namespace juce
