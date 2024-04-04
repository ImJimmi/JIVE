#include "jive_DocumentWindow.h"

namespace jive
{
    DocumentWindow::DocumentWindow()
        : juce::DocumentWindow{
            "",
            juce::LookAndFeel::getDefaultLookAndFeel()
                .findColour(ResizableWindow::backgroundColourId),
            DocumentWindow::allButtons,
        }
        , onCloseButtonPressed{
            [] {
                juce::JUCEApplication::getInstance()->systemRequestedQuit();
            },
        }
    {
#if !JIVE_UNIT_TESTS
        setVisible(true);
#endif
    }

    void DocumentWindow::closeButtonPressed()
    {
        if (onCloseButtonPressed != nullptr)
            onCloseButtonPressed();
    }
} // namespace jive
