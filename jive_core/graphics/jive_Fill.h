#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class Fill
    {
    public:
        Fill();
        explicit Fill(juce::Colour colour);
        explicit Fill(Gradient gradient);

        juce::Optional<juce::Colour> getColour() const;
        void setColour(juce::Colour newColour);

        juce::Optional<Gradient> getGradient() const;
        void setGradient(Gradient newGradient);

    private:
        juce::Optional<juce::Colour> colour;
        juce::Optional<Gradient> gradient;
    };
} // namespace jive

//======================================================================================================================
namespace juce
{
    //==================================================================================================================
    template <>
    class VariantConverter<jive::Fill>
    {
    public:
        //==============================================================================================================
        static jive::Fill fromVar(const var& v);
        static var toVar(const jive::Fill& fill);
    };
} // namespace juce
