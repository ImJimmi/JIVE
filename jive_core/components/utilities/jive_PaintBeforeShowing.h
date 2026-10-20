#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    // Native windows usually appear before their first paint, which happens on
    // a later vblank, so the window shows its empty background for a moment.
    // This paints the window a component is in just before that window is
    // shown, so its first frame already has the component's content.
    class PaintBeforeShowing : private juce::ComponentMovementWatcher
    {
    public:
        explicit PaintBeforeShowing(juce::Component& componentToWatch);

    private:
        void componentMovedOrResized(bool, bool) final;
        void componentPeerChanged() final;
        void componentVisibilityChanged() final;

        JUCE_LEAK_DETECTOR(PaintBeforeShowing)
    };
} // namespace jive
