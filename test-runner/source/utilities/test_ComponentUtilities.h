#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//======================================================================================================================
namespace test
{
    //==================================================================================================================
    std::unique_ptr<juce::Component> createDummyComponent();

    int getPopupMenuNumSeparators(const juce::PopupMenu& menu);
    int getPopupMenuNumHeaders(const juce::PopupMenu& menu);
} // namespace test
