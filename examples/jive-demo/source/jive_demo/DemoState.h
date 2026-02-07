#pragma once

#include <jive_demo/gui/WindowState.h>

namespace jive_demo
{
    class DemoState
    {
    public:
        DemoState() = default;

        [[nodiscard]] WindowState getWindowState()
        {
            return WindowState{
                state.getOrCreateChildWithName("Window", &undoManager),
            };
        }

    private:
        juce::ValueTree state{ "Demo" };
        juce::UndoManager undoManager;
    };
} // namespace jive_demo
