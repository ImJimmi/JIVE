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
        explicit BoxModel(juce::ValueTree sourceState);

        //==============================================================================================================
        float getWidth() const;
        bool hasAutoWidth() const;
        float getHeight() const;
        bool hasAutoHeight() const;
        void setSize(float newWidth, float newHeight);

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
        void invalidateParent();

        //==============================================================================================================
        Length width;
        Length height;
        Length minWidth;
        Length minHeight;
        Property<float> idealWidth;
        Property<float> idealHeight;
        Property<juce::Rectangle<float>> componentSize;
        Property<juce::BorderSize<float>> padding;
        Property<juce::BorderSize<float>> border;
        Property<juce::BorderSize<float>> margin;
        Property<bool> isValid;

        juce::ListenerList<Listener> listeners;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(BoxModel)
    };
} // namespace jive
