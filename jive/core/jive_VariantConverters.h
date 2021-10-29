#pragma once

//======================================================================================================================
namespace juce
{
    //==================================================================================================================
    template <>
    struct VariantConverter<jive::GuiItem::Display>
    {
        //==============================================================================================================
        static jive::GuiItem::Display fromVar(const var& v);
        static var toVar(jive::GuiItem::Display display);

    private:
        //==============================================================================================================
        static const Array<var> varArray;
    };

    //==================================================================================================================
    template <>
    struct VariantConverter<juce::FlexBox::Direction>
    {
        //==============================================================================================================
        static juce::FlexBox::Direction fromVar(const var& v);
        static var toVar(juce::FlexBox::Direction direction);

    private:
        //==============================================================================================================
        static const Array<var> varArray;
    };

    //==================================================================================================================
    template <>
    struct VariantConverter<juce::FlexBox::Wrap>
    {
        //==============================================================================================================
        static juce::FlexBox::Wrap fromVar(const var& v);
        static var toVar(juce::FlexBox::Wrap direction);

    private:
        //==============================================================================================================
        static const Array<var> varArray;
    };

    //==================================================================================================================
    template <>
    struct VariantConverter<juce::FlexBox::JustifyContent>
    {
        //==============================================================================================================
        static juce::FlexBox::JustifyContent fromVar(const var& v);
        static var toVar(juce::FlexBox::JustifyContent direction);

    private:
        //==============================================================================================================
        static const Array<var> varArray;
    };

    //==================================================================================================================
    template <>
    struct VariantConverter<juce::FlexBox::AlignItems>
    {
        //==============================================================================================================
        static juce::FlexBox::AlignItems fromVar(const var& v);
        static var toVar(juce::FlexBox::AlignItems direction);

    private:
        //==============================================================================================================
        static const Array<var> varArray;
    };

    //==================================================================================================================
    template <>
    struct VariantConverter<juce::FlexBox::AlignContent>
    {
        //==============================================================================================================
        static juce::FlexBox::AlignContent fromVar(const var& v);
        static var toVar(juce::FlexBox::AlignContent direction);

    private:
        //==============================================================================================================
        static const Array<var> varArray;
    };

    //==================================================================================================================
    template <>
    struct VariantConverter<juce::FlexItem::AlignSelf>
    {
        //==============================================================================================================
        static juce::FlexItem::AlignSelf fromVar(const var& v);
        static var toVar(juce::FlexItem::AlignSelf alignSelf);

    private:
        //==============================================================================================================
        static const Array<var> varArray;
    };
} // namespace juce
