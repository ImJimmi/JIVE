#include "jive_Fill.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    Fill::Fill()
        : Fill{ juce::Colour{} }
    {
    }

    Fill::Fill(juce::Colour initialColour)
        : colour{ initialColour }
    {
    }

    Fill::Fill(Gradient initialGradient)
        : gradient{ initialGradient }
    {
    }

    //==================================================================================================================
    std::optional<juce::Colour> Fill::getColour() const
    {
        return colour;
    }

    void Fill::setColour(juce::Colour newColour)
    {
        colour = newColour;
        gradient.reset();
    }

    std::optional<Gradient> Fill::getGradient() const
    {
        return gradient;
    }

    void Fill::setGradient(Gradient newGradient)
    {
        gradient = newGradient;
        colour.reset();
    }

    //==================================================================================================================
    bool Fill::operator==(const Fill& other) const
    {
        return colour == other.colour
            && gradient == other.gradient;
    }

    bool Fill::operator!=(const Fill& other) const
    {
        return !(*this == other);
    }
} // namespace jive

//======================================================================================================================
namespace juce
{
    //==================================================================================================================
    jive::Fill VariantConverter<jive::Fill>::fromVar(const var& v)
    {
        if (v.isString())
            return jive::Fill{ VariantConverter<Colour>::fromVar(v) };

        if (v.hasProperty("gradient"))
            return jive::Fill{ VariantConverter<jive::Gradient>::fromVar(v) };

        return jive::Fill{ Colour{} };
    }

    var VariantConverter<jive::Fill>::toVar(const jive::Fill& fill)
    {
        if (fill.getColour().has_value())
            return VariantConverter<Colour>::toVar(*fill.getColour());

        if (fill.getGradient().has_value())
            return VariantConverter<jive::Gradient>::toVar(*fill.getGradient());

        jassertfalse;
        return var{};
    }
} // namespace juce
