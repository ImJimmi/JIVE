#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class Fill
    {
    public:
        //==============================================================================================================
        Fill();
        explicit Fill(juce::Colour colour);
        explicit Fill(Gradient gradient);

        //==============================================================================================================
        std::optional<juce::Colour> getColour() const;
        void setColour(juce::Colour newColour);

        std::optional<Gradient> getGradient() const;
        void setGradient(Gradient newGradient);

        //==============================================================================================================
        bool operator==(const Fill& other) const;
        bool operator!=(const Fill& other) const;

    private:
        //==============================================================================================================
        std::optional<juce::Colour> colour;
        std::optional<Gradient> gradient;
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
