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
        void componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                     bool wasMoved,
                                     bool wasResized) final;

        //==============================================================================================================
        std::shared_ptr<juce::Component> component;
        juce::ValueTree state;

        BackgroundCanvas background;

        Property<juce::var> style;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(StyleSheet)
    };
} // namespace jive
