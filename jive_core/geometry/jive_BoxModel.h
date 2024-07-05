#pragma once

#include "jive_Length.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    class BoxModel
    {
    public:
        struct Listener
        {
            virtual ~Listener() = default;

            virtual void boxModelChanged(BoxModel&) {}
            virtual void boxModelInvalidated(BoxModel&) {}
        };

        /** Prevents any callbacks being invoked on the given BoxModel object
            until the lock instance is destroyed.

            Use with caution! This may cause things to break if the BoxModel
            isn't able to properly respond to vital changes in its given
            ValueTree. However, so long as you're sure you know what you're
            doing, this can be very useful when setting several properties at
            once and you don't want callbacks firing for each one.
        */
        class ScopedCallbackLock
        {
        public:
            explicit ScopedCallbackLock(BoxModel& boxModelToLock);
            ~ScopedCallbackLock();

        private:
            BoxModel& boxModel;
        };

        explicit BoxModel(juce::ValueTree sourceState);

        [[nodiscard]] float getWidth() const;
        void setWidth(float newWidth);
        [[nodiscard]] bool hasAutoWidth() const;
        [[nodiscard]] float getHeight() const;
        void setHeight(float newHeight);
        [[nodiscard]] bool hasAutoHeight() const;
        void setSize(float newWidth, float newHeight);

        [[nodiscard]] juce::BorderSize<float> getPadding() const;
        [[nodiscard]] juce::BorderSize<float> getBorder() const;
        [[nodiscard]] juce::BorderSize<float> getMargin() const;

        /** Returns the outer bounds of this item.
            The outer bounds is the area including the padding and border (but
            excluding the margin), so you can think of this as the sort of
            "physical" bounds of the item, i.e. the bounds of the component.
        */
        [[nodiscard]] juce::Rectangle<float> getOuterBounds() const;

        /** Returns the inner bounds of this item.
            This is the area that the item's content should occupy, i.e. the
            inner-most area within the border and padding.
         */
        [[nodiscard]] juce::Rectangle<float> getContentBounds() const;

        [[nodiscard]] juce::Rectangle<float> getMinimumBounds() const;
        [[nodiscard]] juce::Rectangle<float> getMaximumBounds() const;

        void addListener(Listener& listener) const;
        void removeListener(Listener& listener) const;

        void invalidate();

        juce::ValueTree state;

    private:
        void lock();
        void unlock();

        juce::Rectangle<float> getParentBounds() const;
        void invalidateParent();

        Length width;
        Length height;
        Length minWidth;
        Length minHeight;
        Length maxWidth;
        Length maxHeight;
        Property<float> idealWidth;
        Property<float> idealHeight;
        Property<float> componentWidth;
        Property<float> componentHeight;
        Property<juce::BorderSize<float>> padding;
        Property<juce::BorderSize<float>> border;
        Property<juce::BorderSize<float>> margin;
        Property<bool> isValid;
        Property<bool> callbackLock;

        juce::ListenerList<Listener> listeners;

        JUCE_LEAK_DETECTOR(BoxModel)
    };
} // namespace jive
