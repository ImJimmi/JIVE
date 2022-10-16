#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class StyleSheet : private juce::ComponentListener
    {
    public:
        //==============================================================================================================
        StyleSheet(std::shared_ptr<juce::Component> component, juce::ValueTree state);
        ~StyleSheet();

    private:
        //==============================================================================================================
        struct Style
        {
            Fill background;
        };

        //==============================================================================================================
        void componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                     bool wasMoved,
                                     bool wasResized) final;

        //==============================================================================================================
        juce::FillType getBackgroundFill() const;
        Style collateStyle() const;
        void setStyle(Style style);

        //==============================================================================================================
        std::shared_ptr<juce::Component> component;
        juce::ValueTree state;

        BackgroundCanvas background;

        Property<juce::var, HereditaryValueBehaviour::inheritFromAncestors> style;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(StyleSheet)
    };
} // namespace jive
