#pragma once

#include "jive_GuiItem.h"

#include <juce_core/juce_core.h>

//======================================================================================================================
namespace juce
{
    //==================================================================================================================
    template<>
    struct VariantConverter<jive::GuiItem::Display>
    {
        //==============================================================================================================
        static jive::GuiItem::Display fromVar(const var& v);
        static var toVar (jive::GuiItem::Display display);

    private:
        //==============================================================================================================
        static const Array<var> varArray;
    };

    //==================================================================================================================
    template<>
    struct VariantConverter<juce::FlexBox::Direction>
    {
        //==============================================================================================================
        static juce::FlexBox::Direction fromVar(const var& v);
        static var toVar (juce::FlexBox::Direction direction);

    private:
        //==============================================================================================================
        static const Array<var> varArray;
    };

    //==================================================================================================================
    template<>
    struct VariantConverter<juce::FlexBox::Wrap>
    {
        //==============================================================================================================
        static juce::FlexBox::Wrap fromVar(const var& v);
        static var toVar (juce::FlexBox::Wrap direction);

    private:
        //==============================================================================================================
        static const Array<var> varArray;
    };

    //==================================================================================================================
    template<>
    struct VariantConverter<juce::FlexBox::JustifyContent>
    {
        //==============================================================================================================
        static juce::FlexBox::JustifyContent fromVar(const var& v);
        static var toVar (juce::FlexBox::JustifyContent direction);

    private:
        //==============================================================================================================
        static const Array<var> varArray;
    };
} // namespace juce
