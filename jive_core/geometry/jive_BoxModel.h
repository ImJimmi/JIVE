#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class BoxModel
    {
    public:
        //==============================================================================================================
        struct Listener
        {
            virtual ~Listener() = default;

            virtual void boxModelChanged(BoxModel&) {}
            virtual void boxModelInvalidated(BoxModel&) {}
        };

        //==============================================================================================================
        explicit BoxModel(juce::ValueTree stateSource);

        //==============================================================================================================
        float getWidth() const;
        bool hasAutoWidth() const;
        void setWidth(float newWidth);
        float getHeight() const;
        bool hasAutoHeight() const;
        void setHeight(float newHeight);

        juce::BorderSize<float> getPadding() const;
        juce::BorderSize<float> getBorder() const;
        juce::BorderSize<float> getMargin() const;

        juce::Rectangle<float> getBounds() const;
        juce::Rectangle<float> getMinimumBounds() const;
        juce::Rectangle<float> getContentBounds() const;

        void addListener(Listener& listener) const;
        void removeListener(Listener& listener) const;

        //==============================================================================================================
        juce::ValueTree state;

    private:
        //==============================================================================================================
        juce::Rectangle<float> getParentBounds() const;
        float calculateComponentWidth() const;
        float calculateComponentHeight() const;

        //==============================================================================================================
        Length width;
        Length height;
        Length minWidth;
        Length minHeight;
        Length autoMinWidth;
        Length autoMinHeight;
        Property<float> idealWidth;
        Property<float> idealHeight;
        Property<float> componentWidth;
        Property<float> componentHeight;
        Property<juce::BorderSize<float>> padding;
        Property<juce::BorderSize<float>> border;
        Property<juce::BorderSize<float>> margin;
        Property<bool> isValid;

        std::unique_ptr<BoxModel> parentBoxModel;
        juce::ListenerList<Listener> listeners;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(BoxModel)
    };
} // namespace jive
