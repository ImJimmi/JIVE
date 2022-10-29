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

            virtual void boxModelChanged(BoxModel& boxModel) = 0;
        };

        //==============================================================================================================
        explicit BoxModel(juce::ValueTree stateSource);

        //==============================================================================================================
        float getWidth() const;
        void setWidth(float width);
        bool hasAutoWidth() const;
        float getHeight() const;
        void setHeight(float height);
        bool hasAutoHeight() const;
        void setSize(float width, float height);

        juce::BorderSize<float> getPadding() const;
        juce::BorderSize<float> getBorder() const;
        juce::BorderSize<float> getMargin() const;

        juce::Rectangle<float> getBorderBounds() const;
        juce::Rectangle<float> getPaddingBounds() const;
        juce::Rectangle<float> getContentBounds() const;

        void addListener(Listener& listener) const;
        void removeListener(Listener& listener) const;

        //==============================================================================================================
        juce::ValueTree state;

    private:
        //==============================================================================================================
        Length width;
        Length height;
        TypedValue<juce::BorderSize<float>> padding;
        TypedValue<juce::BorderSize<float>> border;
        TypedValue<juce::BorderSize<float>> margin;
        std::shared_ptr<TypedValue<float>> parentWidth;
        std::shared_ptr<TypedValue<float>> parentHeight;
        TypedValue<float> componentWidth;
        TypedValue<float> componentHeight;
        std::unique_ptr<BoxModel> parentBoxModel;

        juce::ListenerList<Listener> listeners;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(BoxModel)
    };
} // namespace jive
