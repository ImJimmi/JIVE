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

    //==================================================================================================================
    template <>
    struct VariantConverter<Justification>
    {
        //==============================================================================================================
        static Justification fromVar(const var& v);
        static var toVar(Justification justification);
    };

    //==================================================================================================================
    template <typename ValueType>
    struct VariantConverter<BorderSize<ValueType>>
    {
        //==============================================================================================================
        static BorderSize<ValueType> fromVar(const var& v);
        static var toVar(BorderSize<ValueType> border);
    };

    //==================================================================================================================
    template <>
    struct VariantConverter<juce::Identifier>
    {
        //==============================================================================================================
        static juce::Identifier fromVar(const var& v);
        static var toVar(juce::Identifier id);
    };

    //==================================================================================================================
    template <>
    struct VariantConverter<MouseCursor::StandardCursorType>
    {
        //==============================================================================================================
        static MouseCursor::StandardCursorType fromVar(const var& v);
        static var toVar(MouseCursor::StandardCursorType cursor);

    private:
        //==============================================================================================================
        static const Array<var> options;
    };

    //==================================================================================================================
    template <>
    struct VariantConverter<juce::GridItem::JustifySelf>
    {
        //==============================================================================================================
        static juce::GridItem::JustifySelf fromVar(const var& v);
        static var toVar(juce::GridItem::JustifySelf justification);

    private:
        //==============================================================================================================
        static const Array<var> varArray;
    };

    //==================================================================================================================
    template <>
    struct VariantConverter<juce::GridItem::AlignSelf>
    {
        //==============================================================================================================
        static juce::GridItem::AlignSelf fromVar(const var& v);
        static var toVar(juce::GridItem::AlignSelf alignment);

    private:
        //==============================================================================================================
        static const Array<var> varArray;
    };
} // namespace juce
