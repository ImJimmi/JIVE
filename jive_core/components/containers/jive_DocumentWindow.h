#pragma once

#include <jive_core/components/canvases/jive_Canvas.h>
#include <jive_core/components/utilities/jive_PaintBeforeShowing.h>

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    class DocumentWindow : public juce::DocumentWindow
    {
    public:
        DocumentWindow();

        void closeButtonPressed() override;

        std::function<void(void)> onCloseButtonPressed;

    private:
        PaintBeforeShowing paintBeforeShowing{ *this };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DocumentWindow)
    };
} // namespace jive
