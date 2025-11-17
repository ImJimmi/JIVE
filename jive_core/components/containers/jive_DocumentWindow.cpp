#include "jive_DocumentWindow.h"

namespace jive
{
    DocumentWindow::DocumentWindow()
        : juce::DocumentWindow{
            "",
            juce::Colour{ 0xFF4A412A },
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
